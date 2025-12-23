#include "api_http_internal.hpp"
#include "api.hpp"
#include "api_http_common.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/networking.hpp"
#include "../Networking/http2_client.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../Time/time.hpp"
#include <errno.h>
#include <utility>
#include "../Template/move.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#endif

static bool api_http_apply_timeouts(ft_socket &socket_wrapper, int timeout)
{
    if (timeout <= 0)
        return (true);
#ifdef _WIN32
    DWORD win_timeout;

    win_timeout = static_cast<DWORD>(timeout);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&win_timeout), sizeof(win_timeout)) != 0)
        return (false);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&win_timeout), sizeof(win_timeout)) != 0)
        return (false);
#else
    struct timeval tv;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_RCVTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
#endif
    return (true);
}

static bool api_http_is_recoverable_send_error(int error_code)
{
    if (error_code == FT_ERR_SOCKET_SEND_FAILED)
        return (true);
    if (error_code == FT_ERR_IO)
        return (true);
#ifdef _WIN32
    if (error_code == (ft_map_system_error(WSAECONNRESET)))
        return (true);
    if (error_code == (ft_map_system_error(WSAECONNABORTED)))
        return (true);
#else
    if (error_code == (ft_map_system_error(ECONNRESET)))
        return (true);
    if (error_code == (ft_map_system_error(EPIPE)))
        return (true);
#endif
    return (false);
}

bool api_http_should_retry_plain(int error_code)
{
    if (api_http_is_recoverable_send_error(error_code))
        return (true);
    if (error_code == FT_ERR_HTTP_PROTOCOL_MISMATCH)
        return (true);
#ifdef _WIN32
    if (error_code == (ft_map_system_error(WSAECONNRESET)))
        return (true);
#else
    if (error_code == (ft_map_system_error(ECONNRESET)))
        return (true);
#endif
    if (error_code == FT_ERR_SOCKET_RECEIVE_FAILED)
        return (true);
    if (error_code == FT_ERR_SOCKET_CONNECT_FAILED)
        return (true);
    return (false);
}

static bool api_http_http2_failure_requires_eviction(int error_code)
{
    if (error_code == FT_ERR_SOCKET_RECEIVE_FAILED)
        return (true);
    if (error_code == FT_ERR_SOCKET_SEND_FAILED)
        return (true);
    if (error_code == FT_ERR_SOCKET_CONNECT_FAILED)
        return (true);
    if (error_code == FT_ERR_IO)
        return (true);
    return (false);
}

static void api_http_reset_plain_socket(api_connection_pool_handle &connection_handle)
{
    bool should_store;

    should_store = connection_handle.should_store;
    connection_handle.socket.close_socket();
    connection_handle.socket = ft_socket();
    connection_handle.has_socket = false;
    connection_handle.from_pool = false;
    connection_handle.negotiated_http2 = false;
    connection_handle.tls_session = ft_nullptr;
    connection_handle.tls_context = ft_nullptr;
    connection_handle.should_store = should_store;
    connection_handle.plain_socket_timed_out = false;
    connection_handle.plain_socket_validated = false;
    return ;
}

static bool api_http_plain_socket_is_connected(int descriptor)
{
    int result;
    struct sockaddr_storage peer;
#ifdef _WIN32
    int peer_length;
#else
    socklen_t peer_length;
#endif

    if (descriptor < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    ft_bzero(&peer, sizeof(peer));
#ifdef _WIN32
    peer_length = static_cast<int>(sizeof(peer));
    result = getpeername(descriptor, reinterpret_cast<sockaddr*>(&peer), &peer_length);
#else
    peer_length = static_cast<socklen_t>(sizeof(peer));
    result = getpeername(descriptor, reinterpret_cast<sockaddr*>(&peer), &peer_length);
#endif
    if (result == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
    ft_errno = ft_set_errno_from_system_error(errno);
    return (false);
}

bool api_http_plain_socket_is_alive(api_connection_pool_handle &connection_handle)
{
    int poll_descriptor;
    int poll_result;
    char peek_byte;
    ssize_t peek_result;
    int socket_error;

    ft_errno = FT_ERR_SUCCESSS;
    if (connection_handle.plain_socket_timed_out)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    poll_descriptor = connection_handle.socket.get_fd();
    if (poll_descriptor < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    poll_result = nw_poll(&poll_descriptor, 1, ft_nullptr, 0, 50);
    if (poll_result < 0)
    {
        ft_errno = ft_set_errno_from_system_error(errno);
        return (false);
    }
    if (poll_result == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
    if (poll_descriptor == -1)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (false);
    }
    peek_byte = 0;
#ifdef _WIN32
    peek_result = connection_handle.socket.receive_data(&peek_byte, 1, MSG_PEEK);
#else
    peek_result = connection_handle.socket.receive_data(&peek_byte, 1, MSG_PEEK | MSG_DONTWAIT);
#endif
    socket_error = connection_handle.socket.get_error();
    if (peek_result > 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
    if (peek_result == 0)
    {
        bool socket_connected;

        socket_connected = api_http_plain_socket_is_connected(poll_descriptor);
        if (!socket_connected)
        {
            ft_errno = FT_ERR_END_OF_FILE;
            return (false);
        }
#ifdef _WIN32
        if (socket_error == ft_map_system_error(WSAEWOULDBLOCK)
            || socket_error == ft_map_system_error(WSAEINTR))
        {
            ft_errno = FT_ERR_SUCCESSS;
            return (true);
        }
#else
        if (socket_error == ft_map_system_error(EWOULDBLOCK)
            || socket_error == ft_map_system_error(EAGAIN)
            || socket_error == ft_map_system_error(EINTR))
        {
            ft_errno = FT_ERR_SUCCESSS;
            return (true);
        }
#endif
        ft_errno = FT_ERR_END_OF_FILE;
        return (false);
    }
#ifdef _WIN32
    if (socket_error == ft_map_system_error(WSAEWOULDBLOCK)
        || socket_error == ft_map_system_error(WSAEINTR))
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
#else
    if (socket_error == ft_map_system_error(EWOULDBLOCK)
        || socket_error == ft_map_system_error(EAGAIN)
        || socket_error == ft_map_system_error(EINTR))
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
#endif
    ft_errno = socket_error;
    return (false);
}

bool api_http_prepare_plain_socket(
    api_connection_pool_handle &connection_handle, const char *host,
    uint16_t port, int timeout, int &error_code)
{
    bool pooled_connection;

    if (connection_handle.has_socket)
    {
        if (connection_handle.plain_socket_timed_out)
        {
            api_connection_pool_evict(connection_handle);
            api_http_reset_plain_socket(connection_handle);
        }
        if (!connection_handle.plain_socket_validated)
        {
            connection_handle.plain_socket_validated = true;
            connection_handle.plain_socket_timed_out = false;
            return (true);
        }
        if (api_http_plain_socket_is_alive(connection_handle))
        {
            connection_handle.plain_socket_timed_out = false;
            return (true);
        }
        api_http_reset_plain_socket(connection_handle);
    }
    pooled_connection = api_connection_pool_acquire(connection_handle, host, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (pooled_connection)
    {
        if (!api_http_apply_timeouts(connection_handle.socket, timeout))
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error != 0)
                error_code = ft_map_system_error(last_error);
            else
                error_code = FT_ERR_CONFIGURATION;
#else
            if (errno != 0)
                error_code = ft_map_system_error(errno);
            else
                error_code = FT_ERR_CONFIGURATION;
#endif
            api_connection_pool_evict(connection_handle);
            return (false);
        }
        connection_handle.plain_socket_timed_out = false;
        connection_handle.plain_socket_validated = true;
        return (true);
    }
    SocketConfig config;

    config._type = SocketType::CLIENT;
    config._ip = host;
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;
    ft_socket new_socket(config);
    if (new_socket.get_error())
    {
        int socket_error_code;

        socket_error_code = new_socket.get_error();
        if (api_is_configuration_socket_error(socket_error_code))
            error_code = socket_error_code;
        else
            error_code = FT_ERR_SOCKET_CONNECT_FAILED;
        return (false);
    }
    connection_handle.socket = ft_move(new_socket);
    connection_handle.has_socket = true;
    connection_handle.from_pool = false;
    connection_handle.should_store = true;
    connection_handle.security_mode = api_connection_security_mode::PLAIN;
    connection_handle.plain_socket_timed_out = false;
    connection_handle.plain_socket_validated = false;
    return (true);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port, bool &used_http2, int &error_code);

static bool api_http_execute_plain_http2_streaming_once(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &connection_close,
    bool &used_http2, int &error_code);

static char *api_http_finalize_downgrade_response(
    api_connection_pool_handle &connection_handle,
    const ft_string &handshake_buffer, size_t header_length,
    bool chunked_encoding, bool has_length, long long content_length,
    bool connection_close, int *status, int &error_code)
{
    const char *status_line;
    const char *body_start;
    size_t body_length;
    const char *result_source;
    size_t result_length;
    char *result_body;
    ft_string decoded_body;

    status_line = handshake_buffer.c_str();
    if (status)
    {
        const char *space_position;

        *status = -1;
        space_position = ft_strchr(status_line, ' ');
        if (space_position)
            *status = ft_atoi(space_position + 1);
    }
    if (handshake_buffer.size() < header_length)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    body_start = handshake_buffer.c_str() + header_length;
    body_length = handshake_buffer.size() - header_length;
    result_source = body_start;
    result_length = body_length;
    if (chunked_encoding)
    {
        size_t consumed_length;

        consumed_length = 0;
        if (!api_http_decode_chunked(body_start, body_length,
                decoded_body, consumed_length))
        {
            api_connection_pool_disable_store(connection_handle);
            error_code = FT_ERR_IO;
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    else if (has_length)
    {
        size_t expected_length;

        if (content_length < 0)
            expected_length = 0;
        else
            expected_length = static_cast<size_t>(content_length);
        if (body_length < expected_length)
        {
            api_connection_pool_disable_store(connection_handle);
            error_code = FT_ERR_IO;
            return (ft_nullptr);
        }
        result_source = body_start;
        result_length = expected_length;
    }
    else
    {
        result_source = body_start;
        result_length = body_length;
    }
    (void)connection_close;
    api_connection_pool_disable_store(connection_handle);
    api_connection_pool_evict(connection_handle);
    result_body = static_cast<char*>(cma_malloc(result_length + 1));
    if (!result_body)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            error_code = FT_ERR_NO_MEMORY;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    if (result_length > 0)
        ft_memcpy(result_body, result_source, result_length);
    result_body[result_length] = '\0';
    error_code = FT_ERR_SUCCESSS;
    return (result_body);
}
static bool api_http_execute_plain_streaming_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const api_streaming_handler *streaming_handler, bool &connection_close,
    int &error_code);

static bool api_http_send_request(ft_socket &socket_wrapper,
    const ft_string &request, int &error_code)
{
    ssize_t sent;

    if (request.empty())
        return (true);
    sent = socket_wrapper.send_all(request.c_str(), request.size());
    if (sent < 0)
    {
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != FT_ERR_SUCCESSS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_IO;
        return (false);
    }
    return (true);
}

static bool api_http_socket_send_callback(const char *data_pointer,
    size_t data_length, void *context, int &error_code)
{
    ft_socket *socket_pointer;
    ssize_t sent_bytes;

    if (!context)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    socket_pointer = static_cast<ft_socket *>(context);
    if (data_length == 0)
        return (true);
    sent_bytes = socket_pointer->send_all(data_pointer, data_length);
    if (sent_bytes < 0)
    {
        if (socket_pointer->get_error())
            error_code = socket_pointer->get_error();
        else if (ft_errno != FT_ERR_SUCCESSS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_IO;
        return (false);
    }
    return (true);
}

static bool api_http_send_payload(ft_socket &socket_wrapper,
    json_group *payload, int &error_code)
{
    if (!payload)
        return (true);
    if (!api_http_stream_json_payload(payload, api_http_socket_send_callback,
            &socket_wrapper, error_code))
        return (false);
    return (true);
}

static void api_http_map_send_error(int &send_error_code)
{
#ifdef _WIN32
    if (send_error_code == (ft_map_system_error(WSAECONNRESET)))
        send_error_code = FT_ERR_SOCKET_SEND_FAILED;
    if (send_error_code == (ft_map_system_error(WSAECONNABORTED)))
        send_error_code = FT_ERR_SOCKET_SEND_FAILED;
#else
    if (send_error_code == (ft_map_system_error(ECONNRESET)))
        send_error_code = FT_ERR_SOCKET_SEND_FAILED;
    if (send_error_code == (ft_map_system_error(EPIPE)))
        send_error_code = FT_ERR_SOCKET_SEND_FAILED;
#endif
    return ;
}

static bool api_http_prepare_request(const char *method, const char *path,
    const char *host_header, json_group *payload, const char *headers,
    ft_string &request, int &error_code)
{
    size_t payload_length;

    request.clear();
    if (!method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    request = method;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += " ";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += path;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += " HTTP/1.1\r\n";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (!host_header)
        host_header = "";
    request += "Host: ";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += host_header;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (headers && headers[0])
    {
        request += "\r\n";
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
        request += headers;
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
    }
    if (payload)
    {
        if (!api_http_measure_json_payload(payload, payload_length))
        {
            if (ft_errno == FT_ERR_SUCCESSS)
                error_code = FT_ERR_IO;
            else
                error_code = ft_errno;
            return (false);
        }
        request += "\r\nContent-Type: application/json";
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
        if (!api_append_content_length_header(request, payload_length))
        {
            if (ft_errno == FT_ERR_SUCCESSS)
                error_code = FT_ERR_IO;
            else
                error_code = ft_errno;
            return (false);
        }
    }
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    return (true);
}

bool api_http_stream_invoke_body(
    const api_streaming_handler *streaming_handler, const char *chunk_data,
    size_t chunk_size, bool is_final_chunk, int &error_code)
{
    bool callback_result;
    bool invocation_success;

    if (!streaming_handler)
        return (true);
    callback_result = true;
    invocation_success = streaming_handler->invoke_body_callback(chunk_data,
            chunk_size, is_final_chunk, callback_result);
    if (!invocation_success)
    {
        error_code = streaming_handler->get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        return (false);
    }
    if (!callback_result)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    return (true);
}

void api_http_stream_invoke_headers(
    const api_streaming_handler *streaming_handler, int status_code,
    const char *headers)
{
    if (!streaming_handler)
        return ;
    streaming_handler->invoke_headers_callback(status_code, headers);
    return ;
}

bool api_http_stream_process_chunked_buffer(ft_string &buffer,
    long long &chunk_remaining, bool &trailers_pending,
    bool &final_chunk_sent, const api_streaming_handler *streaming_handler,
    int &error_code)
{
    while (true)
    {
        if (trailers_pending)
        {
            const char *trailer_end;
            size_t trailer_length;

            trailer_end = api_http_find_crlf(buffer.c_str(),
                    buffer.c_str() + buffer.size());
            if (!trailer_end)
                break ;
            trailer_length = static_cast<size_t>(trailer_end - buffer.c_str());
            if (buffer.size() < trailer_length + 2)
                break ;
            buffer.erase(0, trailer_length + 2);
            if (trailer_length == 0)
            {
                trailers_pending = false;
                if (!final_chunk_sent)
                {
                    if (!api_http_stream_invoke_body(streaming_handler,
                            ft_nullptr, 0, true, error_code))
                        return (false);
                    final_chunk_sent = true;
                }
            }
            continue ;
        }
        if (chunk_remaining < 0)
        {
            const char *line_end;
            size_t line_length;
            long long chunk_size_value;

            line_end = api_http_find_crlf(buffer.c_str(),
                    buffer.c_str() + buffer.size());
            if (!line_end)
                break ;
            line_length = static_cast<size_t>(line_end - buffer.c_str());
            if (buffer.size() < line_length + 2)
                break ;
            if (!api_http_parse_hex(buffer.c_str(),
                    buffer.c_str() + line_length, chunk_size_value))
            {
                error_code = FT_ERR_IO;
                return (false);
            }
            buffer.erase(0, line_length + 2);
            if (chunk_size_value < 0)
            {
                error_code = FT_ERR_IO;
                return (false);
            }
            if (chunk_size_value == 0)
            {
                chunk_remaining = -1;
                trailers_pending = true;
                continue ;
            }
            chunk_remaining = chunk_size_value;
            continue ;
        }
        size_t required_size;

        required_size = static_cast<size_t>(chunk_remaining);
        if (buffer.size() < required_size + 2)
            break ;
        if (!api_http_stream_invoke_body(streaming_handler,
                buffer.c_str(), required_size, false, error_code))
            return (false);
        buffer.erase(0, required_size);
        if (buffer.size() < 2)
        {
            error_code = FT_ERR_IO;
            return (false);
        }
        if (buffer.c_str()[0] != '\r' || buffer.c_str()[1] != '\n')
        {
            error_code = FT_ERR_IO;
            return (false);
        }
        buffer.erase(0, 2);
        chunk_remaining = -1;
    }
    return (true);
}

static bool api_http_streaming_flush_buffer(ft_string &streaming_body_buffer,
    bool has_length, long long content_length, size_t &streaming_delivered,
    bool &final_chunk_sent, bool chunked_encoding,
    long long &chunk_stream_remaining, bool &chunk_stream_trailers,
    const api_streaming_handler *streaming_handler, int &error_code)
{
    if (streaming_body_buffer.size() == 0)
        return (true);
    if (has_length)
    {
        while (streaming_body_buffer.size() > 0
            && streaming_delivered < static_cast<size_t>(content_length))
        {
            size_t remaining_length;
            size_t chunk_size;
            bool final_chunk;

            remaining_length = static_cast<size_t>(content_length)
                - streaming_delivered;
            chunk_size = streaming_body_buffer.size();
            if (chunk_size > remaining_length)
                chunk_size = remaining_length;
            final_chunk = (streaming_delivered + chunk_size)
                == static_cast<size_t>(content_length);
            if (!api_http_stream_invoke_body(streaming_handler,
                    streaming_body_buffer.c_str(), chunk_size,
                    final_chunk, error_code))
                return (false);
            streaming_body_buffer.erase(0, chunk_size);
            streaming_delivered += chunk_size;
            if (final_chunk)
            {
                final_chunk_sent = true;
                break ;
            }
        }
        if (final_chunk_sent && streaming_body_buffer.size() > 0)
        {
            error_code = FT_ERR_IO;
            return (false);
        }
        return (true);
    }
    if (chunked_encoding)
    {
        if (!api_http_stream_process_chunked_buffer(streaming_body_buffer,
                chunk_stream_remaining, chunk_stream_trailers,
                final_chunk_sent, streaming_handler, error_code))
            return (false);
        if (final_chunk_sent && streaming_body_buffer.size() > 0)
        {
            error_code = FT_ERR_IO;
            return (false);
        }
        return (true);
    }
    if (!api_http_stream_invoke_body(streaming_handler,
            streaming_body_buffer.c_str(), streaming_body_buffer.size(),
            false, error_code))
        return (false);
    streaming_body_buffer.clear();
    return (true);
}

static bool api_http_receive_response(api_connection_pool_handle &connection_handle,
    ft_socket &socket_wrapper,
    ft_string &response, size_t &header_length, bool &connection_close,
    bool &chunked_encoding, bool &has_length, long long &content_length,
    int &error_code, const api_streaming_handler *streaming_handler,
    const ft_string *prefetched_response)
{
    char buffer[32768 + 1];
    ssize_t received;
    bool streaming_enabled;
    bool headers_complete;
    ft_string header_storage;
    ft_string streaming_body_buffer;
    size_t streaming_delivered;
    bool final_chunk_sent;
    int header_status_code;
    long long chunk_stream_remaining;
    bool chunk_stream_trailers;
    size_t prefetched_offset;
    bool use_prefetched;

    streaming_enabled = (streaming_handler != ft_nullptr);
    headers_complete = false;
    streaming_delivered = 0;
    final_chunk_sent = false;
    header_status_code = -1;
    chunk_stream_remaining = -1;
    chunk_stream_trailers = false;
    prefetched_offset = 0;
    use_prefetched = false;
    if (prefetched_response)
        use_prefetched = true;
    response.clear();
    header_length = 0;
    connection_close = false;
    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    connection_handle.plain_socket_timed_out = false;
    while (true)
    {
        received = -1;
        if (use_prefetched)
        {
            size_t prefetched_size;

            prefetched_size = prefetched_response->size();
            if (prefetched_offset < prefetched_size)
            {
                size_t chunk_size;

                chunk_size = prefetched_size - prefetched_offset;
                if (chunk_size > sizeof(buffer) - 1)
                    chunk_size = sizeof(buffer) - 1;
                ft_memcpy(buffer,
                    prefetched_response->c_str() + prefetched_offset,
                    chunk_size);
                buffer[chunk_size] = '\0';
                received = static_cast<ssize_t>(chunk_size);
                prefetched_offset += chunk_size;
                if (prefetched_offset >= prefetched_size)
                    use_prefetched = false;
            }
            else
                use_prefetched = false;
        }
        if (!use_prefetched && received < 0)
            received = socket_wrapper.receive_data(buffer, sizeof(buffer) - 1);
        if (received < 0)
        {
            int socket_error_code;

            socket_error_code = socket_wrapper.get_error();
            if (socket_error_code != FT_ERR_SUCCESSS)
                error_code = socket_error_code;
            else if (ft_errno != FT_ERR_SUCCESSS)
                error_code = ft_errno;
            else
                error_code = FT_ERR_IO;
#ifdef _WIN32
            if (error_code == (ft_map_system_error(WSAEWOULDBLOCK))
                || error_code == (ft_map_system_error(WSAETIMEDOUT)))
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
#else
            if (error_code == (ft_map_system_error(EAGAIN))
                || error_code == (ft_map_system_error(EWOULDBLOCK))
                || error_code == (ft_map_system_error(ETIMEDOUT)))
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
#endif
            if (error_code == FT_ERR_IO && socket_error_code == FT_ERR_SUCCESSS
                && ft_errno == FT_ERR_SUCCESSS)
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
            if (error_code == FT_ERR_SOCKET_RECEIVE_FAILED)
                connection_handle.plain_socket_timed_out = true;
            return (false);
        }
        if (received == 0)
        {
            if (!headers_complete)
            {
                error_code = FT_ERR_IO;
                return (false);
            }
            if (!streaming_enabled)
            {
                size_t body_size;

                body_size = 0;
                if (response.size() > header_length)
                    body_size = response.size() - header_length;
                if (!chunked_encoding && has_length)
                {
                    size_t expected_size;

                    if (content_length <= 0)
                        expected_size = 0;
                    else
                        expected_size = static_cast<size_t>(content_length);
                    if (body_size < expected_size)
                    {
                        error_code = FT_ERR_IO;
                        return (false);
                    }
                }
                if (chunked_encoding)
                {
                    size_t consumed_length;

                    if (!api_http_chunked_body_complete(
                            response.c_str() + header_length,
                            body_size, consumed_length))
                    {
                        error_code = FT_ERR_IO;
                        return (false);
                    }
                }
                connection_close = true;
                break ;
            }
            if (!api_http_streaming_flush_buffer(streaming_body_buffer,
                    has_length, content_length, streaming_delivered,
                    final_chunk_sent, chunked_encoding,
                    chunk_stream_remaining, chunk_stream_trailers,
                    streaming_handler, error_code))
                return (false);
            if (chunked_encoding)
            {
                if (!final_chunk_sent
                    || chunk_stream_trailers
                    || chunk_stream_remaining >= 0
                    || streaming_body_buffer.size() > 0)
                {
                    error_code = FT_ERR_IO;
                    return (false);
                }
                break ;
            }
            if (has_length)
            {
                if (static_cast<long long>(streaming_delivered)
                    != content_length)
                {
                    error_code = FT_ERR_IO;
                    return (false);
                }
                if (!final_chunk_sent)
                {
                    if (!api_http_stream_invoke_body(streaming_handler,
                            ft_nullptr, 0, true, error_code))
                        return (false);
                    final_chunk_sent = true;
                }
                break ;
            }
            if (!api_http_stream_invoke_body(streaming_handler,
                    ft_nullptr, 0, true, error_code))
                return (false);
            final_chunk_sent = true;
            break ;
        }
        if (!headers_complete)
        {
            buffer[received] = '\0';
            response.append(buffer, static_cast<size_t>(received));
            if (response.get_error())
            {
                error_code = response.get_error();
                return (false);
            }
            if (response.size() >= 14)
            {
                if (ft_strncmp(response.c_str(), "PRI * HTTP/2.0", 14) == 0)
                {
                    error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
                    return (false);
                }
            }
            const char *headers_start;
            const char *headers_end;

            headers_start = response.c_str();
            headers_end = ft_strstr(response.c_str(), "\r\n\r\n");
            if (!headers_end)
            {
                if (!streaming_enabled && response.size() >= sizeof(buffer))
                {
                    error_code = FT_ERR_IO;
                    return (false);
                }
                continue ;
            }
            headers_end += 2;
            header_length = static_cast<size_t>(headers_end - headers_start) + 2;
            api_http_parse_headers(headers_start, headers_end,
                connection_close, chunked_encoding, has_length,
                content_length);
            if (!chunked_encoding && !has_length)
                connection_close = true;
            header_storage.assign(response.c_str(), header_length);
            if (header_storage.get_error())
            {
                error_code = header_storage.get_error();
                return (false);
            }
            if (streaming_enabled)
            {
                const char *status_space;
                size_t body_length;

                status_space = ft_strchr(header_storage.c_str(), ' ');
                if (status_space)
                    header_status_code = ft_atoi(status_space + 1);
                api_http_stream_invoke_headers(streaming_handler,
                    header_status_code, header_storage.c_str());
                body_length = response.size() - header_length;
                if (body_length > 0)
                {
                    streaming_body_buffer.append(
                        response.c_str() + header_length, body_length);
                    if (streaming_body_buffer.get_error())
                    {
                        error_code = streaming_body_buffer.get_error();
                        return (false);
                    }
                    if (!api_http_streaming_flush_buffer(
                            streaming_body_buffer, has_length,
                            content_length, streaming_delivered,
                            final_chunk_sent, chunked_encoding,
                            chunk_stream_remaining, chunk_stream_trailers,
                            streaming_handler, error_code))
                        return (false);
                    if (final_chunk_sent)
                        return (true);
                }
                response = header_storage;
            }
            if (!streaming_enabled)
            {
                size_t body_length;

                body_length = response.size() - header_length;
                if (chunked_encoding)
                {
                    size_t consumed_length;

                    if (api_http_chunked_body_complete(
                            response.c_str() + header_length, body_length,
                            consumed_length))
                    {
                        if (consumed_length <= body_length)
                            return (true);
                    }
                }
                else if (has_length)
                {
                    if (body_length >= static_cast<size_t>(content_length))
                        return (true);
                }
                else if (response.size() >= sizeof(buffer))
                {
                    error_code = FT_ERR_IO;
                    return (false);
                }
            }
            headers_complete = true;
            continue ;
        }
        if (!streaming_enabled)
        {
            buffer[received] = '\0';
            response.append(buffer, static_cast<size_t>(received));
            if (response.get_error())
            {
                error_code = response.get_error();
                return (false);
            }
            if (chunked_encoding)
            {
                size_t body_size;
                size_t consumed_length;

                body_size = response.size() - header_length;
                if (api_http_chunked_body_complete(
                        response.c_str() + header_length, body_size,
                        consumed_length))
                {
                    if (consumed_length <= body_size)
                        return (true);
                }
            }
            else if (has_length)
            {
                size_t body_size;

                body_size = response.size() - header_length;
                if (body_size >= static_cast<size_t>(content_length))
                    return (true);
            }
            continue ;
        }
        streaming_body_buffer.append(buffer, static_cast<size_t>(received));
        if (streaming_body_buffer.get_error())
        {
            error_code = streaming_body_buffer.get_error();
            return (false);
        }
        if (!api_http_streaming_flush_buffer(streaming_body_buffer, has_length,
                content_length, streaming_delivered, final_chunk_sent,
                chunked_encoding, chunk_stream_remaining,
                chunk_stream_trailers, streaming_handler, error_code))
            return (false);
        if (final_chunk_sent)
            return (true);
    }
    if (response.size() == 0)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    if (streaming_enabled)
        return (true);
    const char *headers_start;
    const char *headers_end;

    headers_start = response.c_str();
    headers_end = ft_strstr(response.c_str(), "\r\n\r\n");
    if (!headers_end)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    headers_end += 2;
    header_length = static_cast<size_t>(headers_end - headers_start) + 2;
    api_http_parse_headers(headers_start, headers_end, connection_close,
        chunked_encoding, has_length, content_length);
    return (true);
}

static char *api_http_execute_plain_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    int &error_code, const ft_string *prefetched_response,
    bool skip_send)
{
    ft_socket &socket_wrapper = connection_handle.socket;

    if (skip_send && !prefetched_response)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (ft_nullptr);
    }
    if (!api_http_apply_timeouts(socket_wrapper, timeout))
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(last_error);
        else
            error_code = FT_ERR_CONFIGURATION;
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = FT_ERR_CONFIGURATION;
#endif
        return (ft_nullptr);
    }

    bool send_failed;
    int send_error_code;

    send_failed = false;
    send_error_code = FT_ERR_SUCCESSS;
    if (!skip_send)
    {
        ft_string request;

        if (!api_http_prepare_request(method, path, host_header, payload,
                headers, request, error_code))
            return (ft_nullptr);
        if (!api_http_send_request(socket_wrapper, request, error_code))
        {
            send_failed = true;
            send_error_code = error_code;
            api_connection_pool_disable_store(connection_handle);
            api_http_map_send_error(send_error_code);
            if (!api_http_is_recoverable_send_error(send_error_code))
                return (ft_nullptr);
        }
        else if (!api_http_send_payload(socket_wrapper, payload, error_code))
        {
            send_failed = true;
            send_error_code = error_code;
            api_connection_pool_disable_store(connection_handle);
            api_http_map_send_error(send_error_code);
            if (!api_http_is_recoverable_send_error(send_error_code))
                return (ft_nullptr);
        }
    }

    ft_string response;
    size_t header_length;
    bool connection_close;
    bool chunked_encoding;
    bool has_length;
    long long content_length;

    if (send_failed)
        error_code = FT_ERR_SUCCESSS;
    if (!api_http_receive_response(connection_handle, socket_wrapper, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code, ft_nullptr, prefetched_response))
    {
        if (send_failed)
            error_code = send_error_code;
        return (ft_nullptr);
    }
    if (send_failed)
        connection_close = true;
    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    const char *body_start = response.c_str() + header_length;
    size_t body_length = response.size() - header_length;
    ft_string decoded_body;
    const char *result_source;
    size_t result_length;

    result_source = body_start;
    result_length = body_length;
    if (chunked_encoding)
    {
        size_t consumed_length;

        if (!api_http_decode_chunked(body_start, body_length, decoded_body,
                consumed_length))
        {
            error_code = FT_ERR_IO;
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    else if (has_length)
    {
        size_t expected_length;
        size_t index;

        expected_length = static_cast<size_t>(content_length);
        if (body_length < expected_length)
        {
            error_code = FT_ERR_IO;
            return (ft_nullptr);
        }
        decoded_body.clear();
        index = 0;
        while (index < expected_length)
        {
            decoded_body.append(body_start[index]);
            if (decoded_body.get_error())
            {
                error_code = decoded_body.get_error();
                return (ft_nullptr);
            }
            index++;
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    else
    {
        decoded_body.clear();
        decoded_body.append(body_start);
        if (decoded_body.get_error())
        {
            error_code = decoded_body.get_error();
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    if (connection_close)
        api_connection_pool_disable_store(connection_handle);
    char *result_body;

    result_body = static_cast<char*>(cma_malloc(result_length + 1));
    if (!result_body)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            error_code = FT_ERR_NO_MEMORY;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    if (result_length > 0)
        ft_memcpy(result_body, result_source, result_length);
    result_body[result_length] = '\0';
    error_code = FT_ERR_SUCCESSS;
    return (result_body);
}

static bool api_http_execute_plain_streaming_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const api_streaming_handler *streaming_handler, bool &connection_close,
    int &error_code)
{
    ft_socket &socket_wrapper = connection_handle.socket;

    connection_close = false;
    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (false);
    }
    if (!api_http_apply_timeouts(socket_wrapper, timeout))
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(last_error);
        else
            error_code = FT_ERR_CONFIGURATION;
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = FT_ERR_CONFIGURATION;
#endif
        return (false);
    }

    ft_string request;
    if (!api_http_prepare_request(method, path, host_header, payload, headers,
            request, error_code))
        return (false);
    bool send_failed;
    int send_error_code;

    send_failed = false;
    send_error_code = FT_ERR_SUCCESSS;
    if (!api_http_send_request(socket_wrapper, request, error_code))
    {
        send_failed = true;
        send_error_code = error_code;
        api_connection_pool_disable_store(connection_handle);
        api_http_map_send_error(send_error_code);
        if (!api_http_is_recoverable_send_error(send_error_code))
            return (false);
    }
    else if (!api_http_send_payload(socket_wrapper, payload, error_code))
    {
        send_failed = true;
        send_error_code = error_code;
        api_connection_pool_disable_store(connection_handle);
        api_http_map_send_error(send_error_code);
        if (!api_http_is_recoverable_send_error(send_error_code))
            return (false);
    }

    ft_string response;
    size_t header_length;
    bool chunked_encoding;
    bool has_length;
    long long content_length;

    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    if (send_failed)
        error_code = FT_ERR_SUCCESSS;
    if (!api_http_receive_response(connection_handle, socket_wrapper, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code, streaming_handler, ft_nullptr))
    {
        if (send_failed)
            error_code = send_error_code;
        return (false);
    }
    if (send_failed)
        connection_close = true;
    error_code = FT_ERR_SUCCESSS;
    return (true);
}

bool api_http_execute_plain_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    int allowed_attempts;
    bool implicit_retry_added;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    allowed_attempts = max_attempts;
    implicit_retry_added = false;
    while (attempt_index < allowed_attempts)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (false);
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            bool connection_close;
            bool success;

            connection_close = false;
            success = api_http_execute_plain_streaming_once(
                    connection_handle, method, path, host_header, payload,
                    headers, timeout, streaming_handler, connection_close,
                    error_code);
            if (success)
            {
                api_retry_circuit_record_success(connection_handle,
                    retry_policy);
                if (connection_close)
                    api_connection_pool_disable_store(connection_handle);
                return (true);
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (false);
        api_retry_circuit_record_failure(connection_handle, retry_policy);
        if (!implicit_retry_added && retry_policy == ft_nullptr)
        {
            allowed_attempts = 2;
            implicit_retry_added = true;
        }
        api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (attempt_index >= allowed_attempts)
            break ;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    }
    if (error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_IO;
    return (false);
}

static bool api_http_execute_plain_http2_streaming_once(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &connection_close,
    bool &used_http2, int &error_code)
{
    ft_socket &socket_wrapper = connection_handle.socket;
    const char *client_preface;
    ssize_t sent_bytes;
    http2_frame settings_frame;
    ft_string encoded_frame;
    ft_string handshake_buffer;
    bool received_settings_frame;
    bool received_settings_ack;
    bool ack_sent;
    bool handshake_confirmed;
    bool protocol_downgrade;

    (void)retry_policy;
    (void)port;
    connection_close = true;
    used_http2 = false;
    error_code = FT_ERR_SUCCESSS;
    if (!streaming_handler)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (!method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (payload)
    {
        error_code = FT_ERR_UNSUPPORTED_TYPE;
        return (false);
    }
    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (false);
    }
    if (!api_http_apply_timeouts(socket_wrapper, timeout))
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(last_error);
        else
            error_code = FT_ERR_CONFIGURATION;
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = FT_ERR_CONFIGURATION;
#endif
        return (false);
    }
    client_preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    sent_bytes = socket_wrapper.send_all(client_preface, ft_strlen(client_preface));
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
        {
            error_code = socket_wrapper.get_error();
            if (error_code == FT_ERR_IO)
                error_code = FT_ERR_SOCKET_SEND_FAILED;
        }
        else if (ft_errno != FT_ERR_SUCCESSS)
        {
            error_code = ft_errno;
            if (error_code == FT_ERR_IO)
                error_code = FT_ERR_SOCKET_SEND_FAILED;
        }
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        if (error_code == FT_ERR_SOCKET_SEND_FAILED)
            ft_errno = error_code;
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    if (!settings_frame.set_type(0x4))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    if (!settings_frame.set_flags(0x0))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    if (!settings_frame.set_stream_identifier(0))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    settings_frame.clear_payload();
    if (settings_frame.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    if (!http2_encode_frame(settings_frame, encoded_frame, error_code))
    {
        api_connection_pool_disable_store(connection_handle);
        api_connection_pool_evict(connection_handle);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        return (false);
    }
    sent_bytes = socket_wrapper.send_all(encoded_frame.c_str(), encoded_frame.size());
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != FT_ERR_SUCCESSS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    handshake_buffer.clear();
    if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = handshake_buffer.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    received_settings_frame = false;
    received_settings_ack = false;
    ack_sent = false;
    handshake_confirmed = false;
    protocol_downgrade = false;
    while (!received_settings_frame || !received_settings_ack)
    {
        char receive_buffer[1024];
        ssize_t received_bytes;

        received_bytes = socket_wrapper.receive_data(receive_buffer, sizeof(receive_buffer));
        if (received_bytes <= 0)
        {
            api_connection_pool_disable_store(connection_handle);
            if (socket_wrapper.get_error())
                error_code = socket_wrapper.get_error();
            else if (ft_errno != FT_ERR_SUCCESSS)
                error_code = ft_errno;
            else
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
            api_connection_pool_evict(connection_handle);
            return (false);
        }
        handshake_buffer.append(receive_buffer, static_cast<size_t>(received_bytes));
        if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
        {
            api_connection_pool_disable_store(connection_handle);
            error_code = handshake_buffer.get_error();
            api_connection_pool_evict(connection_handle);
            return (false);
        }
        if (handshake_buffer.size() >= 5)
        {
            if (ft_strncmp(handshake_buffer.c_str(), "HTTP/", 5) == 0)
            {
                api_connection_pool_disable_store(connection_handle);
                protocol_downgrade = true;
                break ;
            }
        }
        size_t parse_offset;

        parse_offset = 0;
        while (true)
        {
            http2_frame incoming_frame;
            int frame_error;
            size_t previous_offset;

            previous_offset = parse_offset;
            frame_error = FT_ERR_SUCCESSS;
            if (!http2_decode_frame(reinterpret_cast<const unsigned char*>(handshake_buffer.c_str()),
                    handshake_buffer.size(), parse_offset, incoming_frame, frame_error))
            {
                if (frame_error == FT_ERR_OUT_OF_RANGE)
                {
                    parse_offset = previous_offset;
                    break ;
                }
                api_connection_pool_disable_store(connection_handle);
                error_code = frame_error;
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            uint8_t incoming_type;
            uint8_t incoming_flags;

            if (!incoming_frame.get_type(incoming_type))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (!incoming_frame.get_flags(incoming_flags))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (incoming_type == 0x4)
            {
                if ((incoming_flags & 0x1) != 0)
                    received_settings_ack = true;
                else
                {
                    received_settings_frame = true;
                    handshake_confirmed = true;
                    if (!ack_sent)
                    {
                        http2_frame ack_frame;
                        ft_string ack_encoded;
                        ssize_t ack_bytes;

                        if (!ack_frame.set_type(0x4))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (!ack_frame.set_flags(0x1))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (!ack_frame.set_stream_identifier(0))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        ack_frame.clear_payload();
                        if (ack_frame.get_error() != FT_ERR_SUCCESSS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (!http2_encode_frame(ack_frame, ack_encoded, error_code))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            api_connection_pool_evict(connection_handle);
                            if (error_code == FT_ERR_SUCCESSS)
                                error_code = FT_ERR_IO;
                            return (false);
                        }
                        ack_bytes = socket_wrapper.send_all(ack_encoded.c_str(), ack_encoded.size());
                        if (ack_bytes < 0)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            if (socket_wrapper.get_error())
                                error_code = socket_wrapper.get_error();
                            else if (ft_errno != FT_ERR_SUCCESSS)
                                error_code = ft_errno;
                            else
                                error_code = FT_ERR_SOCKET_SEND_FAILED;
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        ack_sent = true;
                    }
                }
            }
        }
        if (protocol_downgrade)
            break ;
        if (parse_offset > 0)
        {
            handshake_buffer.erase(0, parse_offset);
            if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = handshake_buffer.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
        }
    }
    if (protocol_downgrade)
    {
        api_connection_pool_evict(connection_handle);
        error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
        return (false);
    }
    if (!handshake_confirmed)
    {
        api_connection_pool_disable_store(connection_handle);
        api_connection_pool_evict(connection_handle);
        error_code = FT_ERR_IO;
        return (false);
    }
    used_http2 = true;
    ft_vector<http2_header_field> request_headers;
    http2_header_field header_entry;
    ft_string compressed_headers;
    http2_frame headers_frame;
    ft_string headers_encoded;
    const char *authority_value;

    authority_value = host_header;
    if (!authority_value || authority_value[0] == '\0')
        authority_value = host;
    if (!header_entry.assign_from_cstr(":method", method))
    {
        error_code = header_entry.get_error();
        return (false);
    }
    request_headers.push_back(header_entry);
    if (request_headers.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = request_headers.get_error();
        return (false);
    }
    if (!header_entry.assign_from_cstr(":path", path))
    {
        error_code = header_entry.get_error();
        return (false);
    }
    request_headers.push_back(header_entry);
    if (request_headers.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = request_headers.get_error();
        return (false);
    }
    if (!header_entry.assign_from_cstr(":scheme", "http"))
    {
        error_code = header_entry.get_error();
        return (false);
    }
    request_headers.push_back(header_entry);
    if (request_headers.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = request_headers.get_error();
        return (false);
    }
    if (authority_value && authority_value[0] != '\0')
    {
        if (!header_entry.assign_from_cstr(":authority", authority_value))
        {
            error_code = header_entry.get_error();
            return (false);
        }
        request_headers.push_back(header_entry);
        if (request_headers.get_error() != FT_ERR_SUCCESSS)
        {
            error_code = request_headers.get_error();
            return (false);
        }
    }
    if (headers && headers[0])
    {
        const char *header_cursor;

        header_cursor = headers;
        while (*header_cursor != '\0')
        {
            size_t index;
            ft_string header_name;
            ft_string header_value;

            header_name.clear();
            header_value.clear();
            index = 0;
            while (header_cursor[index] && header_cursor[index] != ':'
                && header_cursor[index] != '\r')
            {
                header_name.append(header_cursor[index]);
                if (header_name.get_error() != FT_ERR_SUCCESSS)
                {
                    error_code = header_name.get_error();
                    return (false);
                }
                index++;
            }
            while (header_cursor[index] == ':' || header_cursor[index] == ' ')
                index++;
            while (header_cursor[index] && header_cursor[index] != '\r'
                && header_cursor[index] != '\n')
            {
                header_value.append(header_cursor[index]);
                if (header_value.get_error() != FT_ERR_SUCCESSS)
                {
                    error_code = header_value.get_error();
                    return (false);
                }
                index++;
            }
            if (header_name.size() > 0)
            {
                if (!header_entry.assign(header_name, header_value))
                {
                    error_code = header_entry.get_error();
                    return (false);
                }
                request_headers.push_back(header_entry);
                if (request_headers.get_error() != FT_ERR_SUCCESSS)
                {
                    error_code = request_headers.get_error();
                    return (false);
                }
            }
            while (header_cursor[index] == '\r' || header_cursor[index] == '\n')
                index++;
            header_cursor += index;
        }
    }
    if (!http2_compress_headers(request_headers, compressed_headers, error_code))
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        return (false);
    }
    if (!headers_frame.set_type(0x1))
    {
        error_code = headers_frame.get_error();
        return (false);
    }
    uint8_t header_flags;

    header_flags = 0x4;
    header_flags |= 0x1;
    if (!headers_frame.set_flags(header_flags))
    {
        error_code = headers_frame.get_error();
        return (false);
    }
    if (!headers_frame.set_stream_identifier(1))
    {
        error_code = headers_frame.get_error();
        return (false);
    }
    if (!headers_frame.set_payload(compressed_headers))
    {
        error_code = headers_frame.get_error();
        return (false);
    }
    if (!http2_encode_frame(headers_frame, headers_encoded, error_code))
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        return (false);
    }
    sent_bytes = socket_wrapper.send_all(headers_encoded.c_str(), headers_encoded.size());
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != FT_ERR_SUCCESSS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    ft_string response_buffer;

    response_buffer = handshake_buffer;
    if (response_buffer.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = response_buffer.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    ft_string header_block;
    ft_vector<http2_header_field> response_headers;
    ft_string header_lines;
    bool end_stream_received;
    bool headers_dispatched;
    bool final_chunk_sent;

    header_block.clear();
    if (header_block.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = header_block.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    response_headers.clear();
    if (response_headers.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = response_headers.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    header_lines.clear();
    if (header_lines.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = header_lines.get_error();
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    end_stream_received = false;
    headers_dispatched = false;
    final_chunk_sent = false;
    while (!end_stream_received)
    {
        size_t parse_offset;
        bool frame_parsed;

        parse_offset = 0;
        frame_parsed = false;
        while (true)
        {
            http2_frame incoming_frame;
            int frame_error;
            size_t previous_offset;

            previous_offset = parse_offset;
            frame_error = FT_ERR_SUCCESSS;
            if (!http2_decode_frame(reinterpret_cast<const unsigned char*>(response_buffer.c_str()),
                    response_buffer.size(), parse_offset, incoming_frame, frame_error))
            {
                if (frame_error == FT_ERR_OUT_OF_RANGE)
                {
                    parse_offset = previous_offset;
                    break ;
                }
                api_connection_pool_disable_store(connection_handle);
                error_code = frame_error;
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            frame_parsed = true;
            uint8_t incoming_type;
            uint8_t incoming_flags;
            uint32_t incoming_stream;
            ft_string payload_copy;

            if (!incoming_frame.get_type(incoming_type))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (!incoming_frame.get_flags(incoming_flags))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (!incoming_frame.get_stream_identifier(incoming_stream))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (!incoming_frame.copy_payload(payload_copy))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (incoming_type == 0x4)
            {
                if ((incoming_flags & 0x1) == 0)
                {
                    http2_frame ack_frame;
                    ft_string ack_buffer;
                    ssize_t ack_sent_bytes;

                    if (!ack_frame.set_type(0x4))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ack_frame.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!ack_frame.set_flags(0x1))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ack_frame.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!ack_frame.set_stream_identifier(0))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ack_frame.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    ack_frame.clear_payload();
                    if (ack_frame.get_error() != FT_ERR_SUCCESSS)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ack_frame.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!http2_encode_frame(ack_frame, ack_buffer, error_code))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        api_connection_pool_evict(connection_handle);
                        if (error_code == FT_ERR_SUCCESSS)
                            error_code = FT_ERR_IO;
                        return (false);
                    }
                    ack_sent_bytes = socket_wrapper.send_all(ack_buffer.c_str(), ack_buffer.size());
                    if (ack_sent_bytes < 0)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        if (socket_wrapper.get_error())
                            error_code = socket_wrapper.get_error();
                        else if (ft_errno != FT_ERR_SUCCESSS)
                            error_code = ft_errno;
                        else
                            error_code = FT_ERR_SOCKET_SEND_FAILED;
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                }
                continue ;
            }
            if (incoming_type == 0x6)
            {
                if ((incoming_flags & 0x1) == 0)
                {
                    http2_frame ping_ack;
                    ft_string ping_buffer;
                    ssize_t ping_sent_bytes;

                    if (!ping_ack.set_type(0x6))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ping_ack.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!ping_ack.set_flags(0x1))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ping_ack.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!ping_ack.set_stream_identifier(0))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ping_ack.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!ping_ack.set_payload(payload_copy))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = ping_ack.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (!http2_encode_frame(ping_ack, ping_buffer, error_code))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        api_connection_pool_evict(connection_handle);
                        if (error_code == FT_ERR_SUCCESSS)
                            error_code = FT_ERR_IO;
                        return (false);
                    }
                    ping_sent_bytes = socket_wrapper.send_all(ping_buffer.c_str(), ping_buffer.size());
                    if (ping_sent_bytes < 0)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        if (socket_wrapper.get_error())
                            error_code = socket_wrapper.get_error();
                        else if (ft_errno != FT_ERR_SUCCESSS)
                            error_code = ft_errno;
                        else
                            error_code = FT_ERR_SOCKET_SEND_FAILED;
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                }
                continue ;
            }
            if (incoming_type == 0x8)
                continue ;
            if (incoming_type == 0x7)
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = FT_ERR_IO;
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            if (incoming_stream != 1)
                continue ;
            if (incoming_type == 0x1 || incoming_type == 0x9)
            {
                header_block.append(payload_copy.c_str(), payload_copy.size());
                if (header_block.get_error() != FT_ERR_SUCCESSS)
                {
                    api_connection_pool_disable_store(connection_handle);
                    error_code = header_block.get_error();
                    api_connection_pool_evict(connection_handle);
                    return (false);
                }
                if ((incoming_flags & 0x4) != 0)
                {
                    int status_code;

                    status_code = -1;
                    if (!http2_decompress_headers(header_block, response_headers, error_code))
                    {
                        if (error_code == FT_ERR_SUCCESSS)
                            error_code = FT_ERR_IO;
                        api_connection_pool_disable_store(connection_handle);
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    header_lines.clear();
                    if (header_lines.get_error() != FT_ERR_SUCCESSS)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = header_lines.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    size_t header_count;
                    size_t header_index;

                    header_count = response_headers.size();
                    if (response_headers.get_error() != FT_ERR_SUCCESSS)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = response_headers.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    header_index = 0;
                    while (header_index < header_count)
                    {
                        const http2_header_field &response_entry = response_headers[header_index];
                        ft_string name_copy;
                        ft_string value_copy;
                        const char *name_cstr;
                        const char *value_cstr;

                        if (response_headers.get_error() != FT_ERR_SUCCESSS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = response_headers.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (!response_entry.copy_name(name_copy))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = response_entry.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (!response_entry.copy_value(value_copy))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = response_entry.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        name_cstr = name_copy.c_str();
                        if (name_copy.get_error() != FT_ERR_SUCCESSS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = name_copy.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        value_cstr = value_copy.c_str();
                        if (value_copy.get_error() != FT_ERR_SUCCESSS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = value_copy.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        if (name_cstr && name_cstr[0] == ':')
                        {
                            if (ft_strcmp(name_cstr, ":status") == 0)
                                status_code = ft_atoi(value_cstr);
                        }
                        else
                        {
                            size_t append_index;

                            append_index = 0;
                            while (name_cstr && name_cstr[append_index])
                            {
                                header_lines.append(name_cstr[append_index]);
                                if (header_lines.get_error() != FT_ERR_SUCCESSS)
                                {
                                    api_connection_pool_disable_store(connection_handle);
                                    error_code = header_lines.get_error();
                                    api_connection_pool_evict(connection_handle);
                                    return (false);
                                }
                                append_index++;
                            }
                            header_lines += ": ";
                            if (header_lines.get_error() != FT_ERR_SUCCESSS)
                            {
                                api_connection_pool_disable_store(connection_handle);
                                error_code = header_lines.get_error();
                                api_connection_pool_evict(connection_handle);
                                return (false);
                            }
                            append_index = 0;
                            while (value_cstr && value_cstr[append_index])
                            {
                                header_lines.append(value_cstr[append_index]);
                                if (header_lines.get_error() != FT_ERR_SUCCESSS)
                                {
                                    api_connection_pool_disable_store(connection_handle);
                                    error_code = header_lines.get_error();
                                    api_connection_pool_evict(connection_handle);
                                    return (false);
                                }
                                append_index++;
                            }
                            header_lines += "\r\n";
                            if (header_lines.get_error() != FT_ERR_SUCCESSS)
                            {
                                api_connection_pool_disable_store(connection_handle);
                                error_code = header_lines.get_error();
                                api_connection_pool_evict(connection_handle);
                                return (false);
                            }
                        }
                        header_index++;
                    }
                    const char *header_text;

                    header_text = header_lines.c_str();
                    if (header_lines.get_error() != FT_ERR_SUCCESSS)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = header_lines.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    api_http_stream_invoke_headers(streaming_handler, status_code,
                        header_text);
                    headers_dispatched = true;
                    header_block.clear();
                    if (header_block.get_error() != FT_ERR_SUCCESSS)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = header_block.get_error();
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                }
                if ((incoming_flags & 0x1) != 0)
                {
                    if (!final_chunk_sent)
                    {
                        if (!api_http_stream_invoke_body(streaming_handler, ft_nullptr,
                                0, true, error_code))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            api_connection_pool_evict(connection_handle);
                            return (false);
                        }
                        final_chunk_sent = true;
                    }
                    end_stream_received = true;
                }
                continue ;
            }
            if (incoming_type == 0x0)
            {
                size_t data_length;
                const char *data_pointer;
                bool final_flag;

                data_length = payload_copy.size();
                if (payload_copy.get_error() != FT_ERR_SUCCESSS)
                {
                    api_connection_pool_disable_store(connection_handle);
                    error_code = payload_copy.get_error();
                    api_connection_pool_evict(connection_handle);
                    return (false);
                }
                data_pointer = payload_copy.c_str();
                if (payload_copy.get_error() != FT_ERR_SUCCESSS)
                {
                    api_connection_pool_disable_store(connection_handle);
                    error_code = payload_copy.get_error();
                    api_connection_pool_evict(connection_handle);
                    return (false);
                }
                final_flag = false;
                if ((incoming_flags & 0x1) != 0)
                    final_flag = true;
                if (data_length > 0 || final_flag)
                {
                    if (!api_http_stream_invoke_body(streaming_handler, data_pointer,
                            data_length, final_flag, error_code))
                    {
                        api_connection_pool_disable_store(connection_handle);
                        api_connection_pool_evict(connection_handle);
                        return (false);
                    }
                    if (final_flag)
                        final_chunk_sent = true;
                }
                if (final_flag)
                    end_stream_received = true;
                continue ;
            }
        }
        if (parse_offset > 0)
        {
            response_buffer.erase(0, parse_offset);
            if (response_buffer.get_error() != FT_ERR_SUCCESSS)
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = response_buffer.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
        }
        if (end_stream_received)
            break ;
        if (!frame_parsed)
        {
            char receive_buffer[16384];
            ssize_t received_bytes;

            received_bytes = socket_wrapper.receive_data(receive_buffer,
                    sizeof(receive_buffer));
            if (received_bytes <= 0)
            {
                api_connection_pool_disable_store(connection_handle);
                if (socket_wrapper.get_error())
                    error_code = socket_wrapper.get_error();
                else if (ft_errno != FT_ERR_SUCCESSS)
                    error_code = ft_errno;
                else
                    error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                api_connection_pool_evict(connection_handle);
                return (false);
            }
            response_buffer.append(receive_buffer,
                static_cast<size_t>(received_bytes));
            if (response_buffer.get_error() != FT_ERR_SUCCESSS)
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = response_buffer.get_error();
                api_connection_pool_evict(connection_handle);
                return (false);
            }
        }
    }
    if (!headers_dispatched)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = FT_ERR_IO;
        api_connection_pool_evict(connection_handle);
        return (false);
    }
    if (!final_chunk_sent)
    {
        if (!api_http_stream_invoke_body(streaming_handler, ft_nullptr, 0, true,
                error_code))
        {
            api_connection_pool_disable_store(connection_handle);
            api_connection_pool_evict(connection_handle);
            return (false);
        }
    }
    api_connection_pool_disable_store(connection_handle);
    api_connection_pool_evict(connection_handle);
    error_code = FT_ERR_SUCCESSS;
    return (true);
}

bool api_http_execute_plain_http2_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &used_http2,
    int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    bool http2_used_local;

    used_http2 = false;
    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    http2_used_local = false;
    while (attempt_index < max_attempts)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (false);
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            bool connection_close;
            bool success;

            connection_close = false;
            http2_used_local = false;
            success = api_http_execute_plain_http2_streaming_once(
                    connection_handle, method, path, host_header, payload,
                    headers, timeout, host, port, retry_policy,
                    streaming_handler, connection_close, http2_used_local,
                    error_code);
            if (success)
            {
                used_http2 = http2_used_local;
                api_retry_circuit_record_success(connection_handle,
                    retry_policy);
                if (connection_close)
                    api_connection_pool_disable_store(connection_handle);
                return (true);
            }
            if (error_code == FT_ERR_HTTP_PROTOCOL_MISMATCH)
                break ;
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (false);
        api_retry_circuit_record_failure(connection_handle, retry_policy);
        api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (attempt_index >= max_attempts)
            break ;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    }
    if (error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_IO;
    return (false);
}

char *api_http_execute_plain_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, bool &used_http2, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    bool http2_used_local;
    bool downgrade_due_to_connect_refused;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    http2_used_local = false;
    downgrade_due_to_connect_refused = false;
    while (attempt_index < max_attempts)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (ft_nullptr);
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            http2_used_local = false;
            char *result_body;

            result_body = api_http_execute_plain_http2_once(connection_handle,
                    method, path, host_header, payload, headers, status,
                    timeout, host, port, http2_used_local, error_code);
            if (result_body)
            {
                used_http2 = http2_used_local;
                api_retry_circuit_record_success(connection_handle,
                    retry_policy);
                return (result_body);
            }
            if (error_code == FT_ERR_HTTP_PROTOCOL_MISMATCH)
                break ;
        }
        else
        {
            bool connect_refused;

            connect_refused = false;
            if (error_code == FT_ERR_SOCKET_CONNECT_FAILED)
                connect_refused = true;
#ifdef _WIN32
            if (error_code == (ft_map_system_error(WSAECONNREFUSED)))
                connect_refused = true;
#else
            if (error_code == (ft_map_system_error(ECONNREFUSED)))
                connect_refused = true;
#endif
            if (connect_refused)
            {
                downgrade_due_to_connect_refused = true;
                api_retry_circuit_record_failure(connection_handle,
                    retry_policy);
                break ;
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            break ;
        if (error_code == FT_ERR_HTTP_PROTOCOL_MISMATCH)
            break ;
        api_retry_circuit_record_failure(connection_handle, retry_policy);
        if (api_http_http2_failure_requires_eviction(error_code))
            api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (attempt_index >= max_attempts)
            break;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay,
                    max_delay);
    }
    if (downgrade_due_to_connect_refused)
    {
        char *fallback_body;

        api_connection_pool_evict(connection_handle);
        fallback_body = api_http_execute_plain(connection_handle, method, path,
                host_header, payload, headers, status, timeout, host, port,
                retry_policy, error_code);
        if (fallback_body)
        {
            used_http2 = false;
            return (fallback_body);
        }
        used_http2 = false;
        return (ft_nullptr);
    }
    used_http2 = false;
    if (error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_IO;
    return (ft_nullptr);
}

char *api_http_execute_plain(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    int last_meaningful_error;
    bool has_meaningful_error;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    last_meaningful_error = FT_ERR_SUCCESSS;
    has_meaningful_error = false;
    while (attempt_index < max_attempts)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (ft_nullptr);
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            char *result_body;

            result_body = api_http_execute_plain_once(connection_handle, method,
                    path, host_header, payload, headers, status, timeout,
                    error_code, ft_nullptr, false);
            if (result_body)
            {
                api_retry_circuit_record_success(connection_handle,
                    retry_policy);
                return (result_body);
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (ft_nullptr);
        api_retry_circuit_record_failure(connection_handle, retry_policy);
        if (error_code != FT_ERR_SOCKET_CONNECT_FAILED
            && error_code != FT_ERR_SUCCESSS)
        {
            last_meaningful_error = error_code;
            has_meaningful_error = true;
        }
        api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (error_code == FT_ERR_SOCKET_CONNECT_FAILED)
        {
            if (attempt_index >= max_attempts)
            {
                if (has_meaningful_error)
                {
                    error_code = last_meaningful_error;
                    ft_errno = last_meaningful_error;
                }
                break ;
            }
            time_sleep_ms(50);
            continue ;
        }
        if (attempt_index >= max_attempts)
            break;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay,
                    max_delay);
    }
    if (error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_IO;
    return (ft_nullptr);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port, bool &used_http2, int &error_code)
{
    ft_socket &socket_wrapper = connection_handle.socket;
    const char *client_preface;
    ssize_t sent_bytes;
    http2_frame settings_frame;
    ft_string encoded_frame;
    ft_string handshake_buffer;
    bool received_settings_frame;
    bool received_settings_ack;
    bool ack_sent;
    bool handshake_confirmed;
    bool protocol_downgrade;

    used_http2 = false;
    error_code = FT_ERR_SUCCESSS;
    if (!method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    (void)host_header;
    (void)payload;
    (void)headers;
    (void)status;
    (void)port;
    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (ft_nullptr);
    }
    if (!api_http_apply_timeouts(socket_wrapper, timeout))
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(last_error);
        else
            error_code = FT_ERR_CONFIGURATION;
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = FT_ERR_CONFIGURATION;
#endif
        return (ft_nullptr);
    }
    client_preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    sent_bytes = socket_wrapper.send_all(client_preface, ft_strlen(client_preface));
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
        {
            error_code = socket_wrapper.get_error();
            if (error_code == FT_ERR_IO)
                error_code = FT_ERR_SOCKET_SEND_FAILED;
        }
        else if (ft_errno != FT_ERR_SUCCESSS)
        {
            error_code = ft_errno;
            if (error_code == FT_ERR_IO)
                error_code = FT_ERR_SOCKET_SEND_FAILED;
        }
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        if (error_code == FT_ERR_SOCKET_SEND_FAILED)
            ft_errno = error_code;
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    if (!settings_frame.set_type(0x4))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    if (!settings_frame.set_flags(0x0))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    if (!settings_frame.set_stream_identifier(0))
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    settings_frame.clear_payload();
    if (settings_frame.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    if (!http2_encode_frame(settings_frame, encoded_frame, error_code))
    {
        api_connection_pool_disable_store(connection_handle);
        api_connection_pool_evict(connection_handle);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    sent_bytes = socket_wrapper.send_all(encoded_frame.c_str(), encoded_frame.size());
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != FT_ERR_SUCCESSS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    handshake_buffer.clear();
    if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = handshake_buffer.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    received_settings_frame = false;
    received_settings_ack = false;
    ack_sent = false;
    handshake_confirmed = false;
    protocol_downgrade = false;
    while (!received_settings_frame || !received_settings_ack)
    {
        char receive_buffer[1024];
        ssize_t received_bytes;

        received_bytes = socket_wrapper.receive_data(receive_buffer, sizeof(receive_buffer));
        if (received_bytes <= 0)
        {
            api_connection_pool_disable_store(connection_handle);
            if (socket_wrapper.get_error())
                error_code = socket_wrapper.get_error();
            else if (ft_errno != FT_ERR_SUCCESSS)
                error_code = ft_errno;
            else
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
            api_connection_pool_evict(connection_handle);
            return (ft_nullptr);
        }
        handshake_buffer.append(receive_buffer, static_cast<size_t>(received_bytes));
        if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
        {
            api_connection_pool_disable_store(connection_handle);
            error_code = handshake_buffer.get_error();
            api_connection_pool_evict(connection_handle);
            return (ft_nullptr);
        }
        if (handshake_buffer.size() >= 5)
        {
            if (ft_strncmp(handshake_buffer.c_str(), "HTTP/", 5) == 0)
            {
                api_connection_pool_disable_store(connection_handle);
                protocol_downgrade = true;
                break ;
            }
        }
        size_t parse_offset;

        parse_offset = 0;
        while (true)
        {
            http2_frame incoming_frame;
            int frame_error;
            size_t previous_offset;

            previous_offset = parse_offset;
            frame_error = FT_ERR_SUCCESSS;
            if (!http2_decode_frame(reinterpret_cast<const unsigned char*>(handshake_buffer.c_str()),
                    handshake_buffer.size(), parse_offset, incoming_frame, frame_error))
            {
                if (frame_error == FT_ERR_OUT_OF_RANGE)
                {
                    parse_offset = previous_offset;
                    break ;
                }
                api_connection_pool_disable_store(connection_handle);
                error_code = frame_error;
                api_connection_pool_evict(connection_handle);
                return (ft_nullptr);
            }
            uint8_t incoming_type;
            uint8_t incoming_flags;

            if (!incoming_frame.get_type(incoming_type))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (ft_nullptr);
            }
            if (!incoming_frame.get_flags(incoming_flags))
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = incoming_frame.get_error();
                api_connection_pool_evict(connection_handle);
                return (ft_nullptr);
            }
            if (incoming_type == 0x4)
            {
                if ((incoming_flags & 0x1) != 0)
                    received_settings_ack = true;
                else
                {
                    received_settings_frame = true;
                    handshake_confirmed = true;
                    if (!ack_sent)
                    {
                        http2_frame ack_frame;
                        ft_string ack_encoded;
                        ssize_t ack_bytes;

                        if (!ack_frame.set_type(0x4))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        if (!ack_frame.set_flags(0x1))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        if (!ack_frame.set_stream_identifier(0))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        ack_frame.clear_payload();
                        if (ack_frame.get_error() != FT_ERR_SUCCESSS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        if (!http2_encode_frame(ack_frame, ack_encoded, error_code))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            api_connection_pool_evict(connection_handle);
                            if (error_code == FT_ERR_SUCCESSS)
                                error_code = FT_ERR_IO;
                            return (ft_nullptr);
                        }
                        ack_bytes = socket_wrapper.send_all(ack_encoded.c_str(), ack_encoded.size());
                        if (ack_bytes < 0)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            if (socket_wrapper.get_error())
                                error_code = socket_wrapper.get_error();
                            else if (ft_errno != FT_ERR_SUCCESSS)
                                error_code = ft_errno;
                            else
                                error_code = FT_ERR_SOCKET_SEND_FAILED;
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        ack_sent = true;
                    }
                }
            }
        }
        if (protocol_downgrade)
            break ;
        if (parse_offset > 0)
            handshake_buffer.erase(0, parse_offset);
    }
    if (protocol_downgrade)
    {
        char *fallback_body;
        bool downgrade_headers_ready;
        bool downgrade_complete;
        size_t downgrade_header_length;
        bool downgrade_chunked_encoding;
        bool downgrade_has_length;
        long long downgrade_content_length;
        bool downgrade_connection_close;
        bool downgrade_fully_buffered;

        downgrade_headers_ready = false;
        downgrade_complete = false;
        downgrade_header_length = 0;
        downgrade_chunked_encoding = false;
        downgrade_has_length = false;
        downgrade_content_length = 0;
        downgrade_connection_close = false;
        downgrade_fully_buffered = false;
        while (!downgrade_complete)
        {
            const char *headers_start;
            const char *headers_end;
            bool body_ready;

            headers_start = handshake_buffer.c_str();
            headers_end = ft_strstr(handshake_buffer.c_str(), "\r\n\r\n");
            if (headers_end)
            {
                headers_end += 2;
                downgrade_header_length = static_cast<size_t>(headers_end - headers_start) + 2;
                if (!downgrade_headers_ready)
                {
                    downgrade_headers_ready = true;
                    api_http_parse_headers(headers_start, headers_end,
                        downgrade_connection_close, downgrade_chunked_encoding,
                        downgrade_has_length, downgrade_content_length);
                }
            }
            body_ready = false;
            if (downgrade_headers_ready)
            {
                if (downgrade_chunked_encoding)
                {
                    size_t body_size;
                    size_t consumed_length;

                    body_size = handshake_buffer.size() - downgrade_header_length;
                    if (body_size > 0)
                    {
                        consumed_length = 0;
                        if (api_http_chunked_body_complete(
                                handshake_buffer.c_str() + downgrade_header_length,
                                body_size, consumed_length))
                        {
                            if (consumed_length <= body_size)
                                body_ready = true;
                        }
                    }
                }
                else if (downgrade_has_length)
                {
                    if (downgrade_content_length <= 0)
                        body_ready = true;
                    else if (handshake_buffer.size()
                        >= downgrade_header_length
                            + static_cast<size_t>(downgrade_content_length))
                        body_ready = true;
                }
                else
                    body_ready = true;
            }
            if (downgrade_headers_ready && body_ready)
            {
                downgrade_connection_close = true;
                downgrade_complete = true;
                downgrade_fully_buffered = true;
                break ;
            }
            char receive_buffer[1024];
            ssize_t received_bytes;

            received_bytes = socket_wrapper.receive_data(receive_buffer, sizeof(receive_buffer));
            if (received_bytes < 0)
            {
                api_connection_pool_disable_store(connection_handle);
                if (socket_wrapper.get_error())
                    error_code = socket_wrapper.get_error();
                else if (ft_errno != FT_ERR_SUCCESSS)
                    error_code = ft_errno;
                else
                    error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                api_connection_pool_evict(connection_handle);
                return (ft_nullptr);
            }
            if (received_bytes == 0)
            {
                if (!downgrade_headers_ready)
                {
                    api_connection_pool_disable_store(connection_handle);
                    error_code = FT_ERR_IO;
                    api_connection_pool_evict(connection_handle);
                    return (ft_nullptr);
                }
                if (!body_ready)
                {
                    if (downgrade_chunked_encoding)
                    {
                        api_connection_pool_disable_store(connection_handle);
                        error_code = FT_ERR_IO;
                        api_connection_pool_evict(connection_handle);
                        return (ft_nullptr);
                    }
                    if (downgrade_has_length)
                    {
                        size_t expected_length;
                        size_t body_size;

                        expected_length = 0;
                        if (downgrade_content_length > 0)
                            expected_length = static_cast<size_t>(downgrade_content_length);
                        body_size = 0;
                        if (handshake_buffer.size() > downgrade_header_length)
                            body_size = handshake_buffer.size() - downgrade_header_length;
                        if (body_size < expected_length)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = FT_ERR_IO;
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        body_ready = true;
                    }
                }
                downgrade_connection_close = true;
                downgrade_complete = true;
                if (body_ready)
                    downgrade_fully_buffered = true;
                break ;
            }
            handshake_buffer.append(receive_buffer, static_cast<size_t>(received_bytes));
            if (handshake_buffer.get_error() != FT_ERR_SUCCESSS)
            {
                api_connection_pool_disable_store(connection_handle);
                error_code = handshake_buffer.get_error();
                api_connection_pool_evict(connection_handle);
                return (ft_nullptr);
            }
        }

        if (downgrade_complete)
        {
            int fallback_error_code;

            if (downgrade_fully_buffered)
            {
                fallback_body = api_http_finalize_downgrade_response(connection_handle,
                        handshake_buffer, downgrade_header_length,
                        downgrade_chunked_encoding, downgrade_has_length,
                        downgrade_content_length, downgrade_connection_close,
                        status, error_code);
                if (fallback_body)
                {
                    used_http2 = false;
                    return (fallback_body);
                }
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
            }
            else
            {
                fallback_body = api_http_execute_plain_once(connection_handle, method,
                        path, host_header, payload, headers, status, timeout,
                        error_code, &handshake_buffer, true);
                if (fallback_body)
                {
                    used_http2 = false;
                    return (fallback_body);
                }
                api_connection_pool_evict(connection_handle);
                fallback_error_code = error_code;
                fallback_body = api_http_finalize_downgrade_response(connection_handle,
                        handshake_buffer, downgrade_header_length,
                        downgrade_chunked_encoding, downgrade_has_length,
                        downgrade_content_length, downgrade_connection_close,
                        status, error_code);
                if (fallback_body)
                {
                    used_http2 = false;
                    return (fallback_body);
                }
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = fallback_error_code;
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
            }
        }
        api_connection_pool_evict(connection_handle);
        if (!api_http_prepare_plain_socket(connection_handle, host, port, timeout,
                error_code))
        {
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
            api_connection_pool_disable_store(connection_handle);
            return (ft_nullptr);
        }
        fallback_body = api_http_execute_plain_once(connection_handle, method,
                path, host_header, payload, headers, status, timeout,
                error_code, ft_nullptr, false);
        if (fallback_body)
        {
            used_http2 = false;
            return (fallback_body);
        }
        api_connection_pool_evict(connection_handle);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
        api_connection_pool_disable_store(connection_handle);
        return (ft_nullptr);
    }
    api_connection_pool_disable_store(connection_handle);
    api_connection_pool_evict(connection_handle);
    if (handshake_confirmed)
        used_http2 = true;
    error_code = FT_ERR_UNSUPPORTED_TYPE;
    return (ft_nullptr);
}
