#include "api_http_internal.hpp"
#include "api.hpp"
#include "api_http_common.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/http2_client.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../Time/time.hpp"
#include <errno.h>
#include <utility>
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

static bool api_http_should_retry_plain(int error_code)
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

bool api_http_prepare_plain_socket(
    api_connection_pool_handle &connection_handle, const char *host,
    uint16_t port, int timeout, int &error_code)
{
    bool pooled_connection;

    if (connection_handle.has_socket)
        return (true);
    pooled_connection = api_connection_pool_acquire(connection_handle, host, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (pooled_connection)
        return (true);
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
    connection_handle.socket = std::move(new_socket);
    connection_handle.has_socket = true;
    connection_handle.from_pool = false;
    connection_handle.should_store = true;
    connection_handle.security_mode = api_connection_security_mode::PLAIN;
    return (true);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
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
    ft_string decoded_body;
    const char *result_source;
    size_t result_length;
    char *result_body;

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
        size_t index;

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
        decoded_body.clear();
        index = 0;
        while (index < expected_length)
        {
            decoded_body.append(body_start[index]);
            if (decoded_body.get_error())
            {
                api_connection_pool_disable_store(connection_handle);
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
            api_connection_pool_disable_store(connection_handle);
            error_code = decoded_body.get_error();
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    api_connection_pool_disable_store(connection_handle);
    if (connection_close)
        api_connection_pool_evict(connection_handle);
    result_body = static_cast<char*>(cma_malloc(result_length + 1));
    if (!result_body)
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_ERR_NO_MEMORY;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    if (result_length > 0)
        ft_memcpy(result_body, result_source, result_length);
    result_body[result_length] = '\0';
    error_code = ER_SUCCESS;
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
        else if (ft_errno != ER_SUCCESS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_IO;
        return (false);
    }
    return (true);
}

static bool api_http_prepare_request(const char *method, const char *path,
    const char *host_header, json_group *payload, const char *headers,
    ft_string &request, ft_string &body_string, int &error_code)
{
    char *temporary_string;

    request.clear();
    body_string.clear();
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
        temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            if (ft_errno == ER_SUCCESS)
                error_code = FT_ERR_NO_MEMORY;
            else
                error_code = ft_errno;
            return (false);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        if (body_string.get_error())
        {
            error_code = body_string.get_error();
            return (false);
        }
        request += "\r\nContent-Type: application/json";
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
        if (!api_append_content_length_header(request, body_string.size()))
        {
            if (ft_errno == ER_SUCCESS)
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
    if (payload)
    {
        request += body_string.c_str();
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
    }
    return (true);
}

bool api_http_stream_invoke_body(
    const api_streaming_handler *streaming_handler, const char *chunk_data,
    size_t chunk_size, bool is_final_chunk, int &error_code)
{
    bool callback_result;

    if (!streaming_handler)
        return (true);
    if (!streaming_handler->body_callback)
        return (true);
    callback_result = streaming_handler->body_callback(chunk_data, chunk_size,
            is_final_chunk, streaming_handler->user_data);
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
    if (!streaming_handler->headers_callback)
        return ;
    streaming_handler->headers_callback(status_code, headers,
        streaming_handler->user_data);
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

static bool api_http_receive_response(ft_socket &socket_wrapper,
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
            if (socket_error_code != ER_SUCCESS)
                error_code = socket_error_code;
            else if (ft_errno != ER_SUCCESS)
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
            if (error_code == FT_ERR_IO && socket_error_code == ER_SUCCESS
                && ft_errno == ER_SUCCESS)
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
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
                if (!chunked_encoding && has_length)
                {
                    size_t expected_size;

                    expected_size = static_cast<size_t>(content_length);
                    if (response.size() < header_length + expected_size)
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
                            response.size() - header_length, consumed_length))
                    {
                        error_code = FT_ERR_IO;
                        return (false);
                    }
                }
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
    send_error_code = ER_SUCCESS;
    if (!skip_send)
    {
        ft_string request;
        ft_string body_string;

        if (!api_http_prepare_request(method, path, host_header, payload,
                headers, request, body_string, error_code))
            return (ft_nullptr);
        if (!api_http_send_request(socket_wrapper, request, error_code))
        {
            send_failed = true;
            send_error_code = error_code;
            api_connection_pool_disable_store(connection_handle);
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
        error_code = ER_SUCCESS;
    if (!api_http_receive_response(socket_wrapper, response, header_length,
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
        if (ft_errno == ER_SUCCESS)
            error_code = FT_ERR_NO_MEMORY;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    if (result_length > 0)
        ft_memcpy(result_body, result_source, result_length);
    result_body[result_length] = '\0';
    error_code = ER_SUCCESS;
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
    ft_string body_string;
    if (!api_http_prepare_request(method, path, host_header, payload, headers,
            request, body_string, error_code))
        return (false);
    bool send_failed;
    int send_error_code;

    send_failed = false;
    send_error_code = ER_SUCCESS;
    if (!api_http_send_request(socket_wrapper, request, error_code))
    {
        send_failed = true;
        send_error_code = error_code;
        api_connection_pool_disable_store(connection_handle);
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
        error_code = ER_SUCCESS;
    if (!api_http_receive_response(socket_wrapper, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code, streaming_handler, ft_nullptr))
    {
        if (send_failed)
            error_code = send_error_code;
        return (false);
    }
    if (send_failed)
        connection_close = true;
    error_code = ER_SUCCESS;
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
                if (connection_close)
                    api_connection_pool_disable_store(connection_handle);
                return (true);
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (false);
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
    if (error_code == ER_SUCCESS)
        error_code = FT_ERR_IO;
    return (false);
}

bool api_http_execute_plain_http2_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &used_http2,
    int &error_code)
{
    used_http2 = false;
    return (api_http_execute_plain_streaming(connection_handle, method, path,
            host_header, payload, headers, timeout, host, port, retry_policy,
            streaming_handler, error_code));
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
                    timeout, http2_used_local, error_code);
            if (result_body)
            {
                used_http2 = http2_used_local;
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
                break ;
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            break ;
        if (error_code == FT_ERR_HTTP_PROTOCOL_MISMATCH)
            break ;
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
    if (error_code == ER_SUCCESS)
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

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    while (attempt_index < max_attempts)
    {
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
                return (result_body);
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (ft_nullptr);
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
    if (error_code == ER_SUCCESS)
        error_code = FT_ERR_IO;
    return (ft_nullptr);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
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

    used_http2 = false;
    error_code = ER_SUCCESS;
    if (!method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    (void)host_header;
    (void)payload;
    (void)headers;
    (void)status;
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
            error_code = socket_wrapper.get_error();
        else if (ft_errno != ER_SUCCESS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    settings_frame.type = 0x4;
    settings_frame.flags = 0x0;
    settings_frame.stream_id = 0;
    settings_frame.payload.clear();
    if (settings_frame.payload.get_error() != ER_SUCCESS)
    {
        api_connection_pool_disable_store(connection_handle);
        error_code = settings_frame.payload.get_error();
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    if (!http2_encode_frame(settings_frame, encoded_frame, error_code))
    {
        api_connection_pool_disable_store(connection_handle);
        api_connection_pool_evict(connection_handle);
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    sent_bytes = socket_wrapper.send_all(encoded_frame.c_str(), encoded_frame.size());
    if (sent_bytes < 0)
    {
        api_connection_pool_disable_store(connection_handle);
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != ER_SUCCESS)
            error_code = ft_errno;
        else
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        api_connection_pool_evict(connection_handle);
        return (ft_nullptr);
    }
    handshake_buffer.clear();
    if (handshake_buffer.get_error() != ER_SUCCESS)
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
            else if (ft_errno != ER_SUCCESS)
                error_code = ft_errno;
            else
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
            api_connection_pool_evict(connection_handle);
            return (ft_nullptr);
        }
        handshake_buffer.append(receive_buffer, static_cast<size_t>(received_bytes));
        if (handshake_buffer.get_error() != ER_SUCCESS)
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
            frame_error = ER_SUCCESS;
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
            if (incoming_frame.type == 0x4)
            {
                if ((incoming_frame.flags & 0x1) != 0)
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

                        ack_frame.type = 0x4;
                        ack_frame.flags = 0x1;
                        ack_frame.stream_id = 0;
                        ack_frame.payload.clear();
                        if (ack_frame.payload.get_error() != ER_SUCCESS)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = ack_frame.payload.get_error();
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                        if (!http2_encode_frame(ack_frame, ack_encoded, error_code))
                        {
                            api_connection_pool_disable_store(connection_handle);
                            api_connection_pool_evict(connection_handle);
                            if (error_code == ER_SUCCESS)
                                error_code = FT_ERR_IO;
                            return (ft_nullptr);
                        }
                        ack_bytes = socket_wrapper.send_all(ack_encoded.c_str(), ack_encoded.size());
                        if (ack_bytes < 0)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            if (socket_wrapper.get_error())
                                error_code = socket_wrapper.get_error();
                            else if (ft_errno != ER_SUCCESS)
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

        downgrade_headers_ready = false;
        downgrade_complete = false;
        downgrade_header_length = 0;
        downgrade_chunked_encoding = false;
        downgrade_has_length = false;
        downgrade_content_length = 0;
        downgrade_connection_close = false;
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
                downgrade_complete = true;
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
                else if (ft_errno != ER_SUCCESS)
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

                        expected_length = 0;
                        if (downgrade_content_length > 0)
                            expected_length = static_cast<size_t>(downgrade_content_length);
                        if (handshake_buffer.size() < downgrade_header_length + expected_length)
                        {
                            api_connection_pool_disable_store(connection_handle);
                            error_code = FT_ERR_IO;
                            api_connection_pool_evict(connection_handle);
                            return (ft_nullptr);
                        }
                    }
                }
                downgrade_complete = true;
                break ;
            }
            handshake_buffer.append(receive_buffer, static_cast<size_t>(received_bytes));
            if (handshake_buffer.get_error() != ER_SUCCESS)
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

            fallback_body = api_http_execute_plain_once(connection_handle, method,
                    path, host_header, payload, headers, status, timeout,
                    error_code, &handshake_buffer, true);
            if (fallback_body)
            {
                used_http2 = false;
                return (fallback_body);
            }
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
            if (error_code == ER_SUCCESS)
                error_code = fallback_error_code;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_HTTP_PROTOCOL_MISMATCH;
            api_connection_pool_disable_store(connection_handle);
            return (ft_nullptr);
        }
        fallback_body = api_http_execute_plain_once(connection_handle, method,
                path, host_header, payload, headers, status, timeout,
                error_code, &handshake_buffer, false);
        if (fallback_body)
        {
            used_http2 = false;
            return (fallback_body);
        }
        if (error_code == ER_SUCCESS)
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
