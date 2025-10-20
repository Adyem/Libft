#include "api_http_internal.hpp"
#include "api.hpp"
#include "api_http_common.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Networking/networking.hpp"
#include "../Networking/http2_client.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../Time/time.hpp"
#include <errno.h>
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
#include <openssl/err.h>
#include <cstdint>
#include <climits>
#include <utility>

static bool api_https_prepare_request(const char *method, const char *path,
    const char *host_header, json_group *payload, const char *headers,
    ft_string &request, ft_string &body_string, int &error_code);

void api_request_set_ssl_error(SSL *ssl_session, int operation_result)
{
    unsigned long library_error;

    library_error = ERR_get_error();
    if (library_error != 0)
    {
        if (library_error > static_cast<unsigned long>(INT_MAX))
            ft_errno = INT_MAX;
        else
            ft_errno = static_cast<int>(library_error);
        return ;
    }
    if (ssl_session)
    {
        int ssl_error;

        ssl_error = SSL_get_error(ssl_session, operation_result);
        if (ssl_error == SSL_ERROR_SYSCALL)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error != 0)
            {
                ft_errno = ft_map_system_error(last_error);
                return ;
            }
#endif
            if (errno != 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
            ft_errno = FT_ERR_SSL_SYSCALL_ERROR;
            return ;
        }
        ft_errno = ssl_error;
        return ;
    }
    ft_errno = FT_ERR_IO;
    return ;
}

static bool ssl_pointer_supports_network_checks(SSL *ssl)
{
    uintptr_t ssl_address;
    static const uintptr_t minimum_valid_address = 0x1000;

    if (ssl == ft_nullptr)
        return (false);
    ssl_address = reinterpret_cast<uintptr_t>(ssl);
    if (ssl_address < minimum_valid_address)
        return (false);
    if ((ssl_address & (sizeof(void *) - 1)) != 0)
        return (false);
    return (true);
}

static ssize_t ssl_send_all(SSL *ssl, const void *data, size_t size)
{
    size_t total;
    const char *ptr;

    total = 0;
    ptr = static_cast<const char*>(data);
    while (total < size)
    {
        ssize_t sent;

        sent = nw_ssl_write(ssl, ptr + total, size - total);
        if (sent > 0)
        {
            total += static_cast<size_t>(sent);
            continue ;
        }
        if (sent < 0)
            return (-1);
        if (ft_errno == FT_ERR_SSL_WANT_READ || ft_errno == FT_ERR_SSL_WANT_WRITE)
        {
            if (ssl_pointer_supports_network_checks(ssl))
            {
                if (networking_check_ssl_after_send(ssl) != 0)
                    return (-1);
            }
            continue ;
        }
        return (-1);
    }
    return (static_cast<ssize_t>(total));
}

static bool api_http_apply_timeouts(ft_socket &socket_wrapper, int timeout)
{
    int file_descriptor;

    if (timeout <= 0)
        return (true);
    file_descriptor = socket_wrapper.get_fd();
    if (file_descriptor < 0)
        return (false);
#ifdef _WIN32
    DWORD timeout_value;

    timeout_value = static_cast<DWORD>(timeout);
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char *>(&timeout_value), sizeof(timeout_value)) != 0)
        return (false);
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char *>(&timeout_value), sizeof(timeout_value)) != 0)
        return (false);
#else
    struct timeval tv;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_RCVTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_SNDTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
#endif
    return (true);
}

static bool api_https_should_retry(int error_code)
{
    if (error_code == FT_ERR_SOCKET_SEND_FAILED)
        return (true);
    if (error_code == FT_ERR_SOCKET_RECEIVE_FAILED)
        return (true);
#ifdef _WIN32
    if (error_code == (ft_map_system_error(WSAECONNRESET)))
        return (true);
#else
    if (error_code == (ft_map_system_error(ECONNRESET)))
        return (true);
#endif
    if (error_code == FT_ERR_SOCKET_CONNECT_FAILED)
        return (true);
    return (false);
}

static bool api_https_prepare_socket(api_connection_pool_handle &connection_handle,
    const char *host, uint16_t port, int timeout,
    const char *security_identity, int &error_code)
{
    bool pooled_connection;

    if (connection_handle.has_socket)
        return (true);
    pooled_connection = api_connection_pool_acquire(connection_handle, host, port,
            api_connection_security_mode::TLS, security_identity);
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
    connection_handle.security_mode = api_connection_security_mode::TLS;
    connection_handle.tls_session = ft_nullptr;
    connection_handle.tls_context = ft_nullptr;
    return (true);
}

static char *api_https_execute_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, bool &used_http2,
    int &error_code);

static bool api_https_send_request(SSL *ssl_session, const ft_string &request,
    int &error_code)
{
    if (!ssl_session)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (request.empty())
        return (true);
    if (ssl_send_all(ssl_session, request.c_str(), request.size()) < 0)
    {
        if (ft_errno == ER_SUCCESS)
        {
            api_request_set_ssl_error(ssl_session, -1);
            error_code = ft_errno;
        }
        else
            error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        return (false);
    }
    return (true);
}

static bool api_https_ensure_session(
    api_connection_pool_handle &connection_handle, int timeout,
    const char *ca_certificate, bool verify_peer, int &error_code)
{
    ft_socket &socket_wrapper = connection_handle.socket;

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
    if (connection_handle.tls_session)
        return (true);
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        return (false);
    }
    SSL_CTX *context;

    context = SSL_CTX_new(TLS_client_method());
    if (!context)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        return (false);
    }
    if (verify_peer)
    {
        if (ca_certificate && ca_certificate[0] != '\0')
        {
            if (SSL_CTX_load_verify_locations(context, ca_certificate,
                    ft_nullptr) != 1)
            {
                api_request_set_ssl_error(ft_nullptr, 0);
                error_code = ft_errno;
                SSL_CTX_free(context);
                return (false);
            }
        }
        else
        {
            if (SSL_CTX_set_default_verify_paths(context) != 1)
            {
                api_request_set_ssl_error(ft_nullptr, 0);
                error_code = ft_errno;
                SSL_CTX_free(context);
                return (false);
            }
        }
        SSL_CTX_set_verify(context, SSL_VERIFY_PEER, ft_nullptr);
    }
    else
        SSL_CTX_set_verify(context, SSL_VERIFY_NONE, ft_nullptr);
    SSL *ssl_session;

    ssl_session = SSL_new(context);
    if (!ssl_session)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        SSL_CTX_free(context);
        return (false);
    }
    if (SSL_set_fd(ssl_session, socket_wrapper.get_fd()) != 1)
    {
        api_request_set_ssl_error(ssl_session, 0);
        error_code = ft_errno;
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
        SSL_CTX_free(context);
        return (false);
    }
    int ssl_connect_result;

    ssl_connect_result = SSL_connect(ssl_session);
    if (ssl_connect_result <= 0)
    {
        api_request_set_ssl_error(ssl_session, ssl_connect_result);
        error_code = ft_errno;
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
        SSL_CTX_free(context);
        return (false);
    }
    connection_handle.tls_context = context;
    connection_handle.tls_session = ssl_session;
    if (!api_connection_pool_track_tls_session(connection_handle.tls_session))
    {
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
        SSL_CTX_free(context);
        connection_handle.tls_context = ft_nullptr;
        connection_handle.tls_session = ft_nullptr;
        error_code = FT_ERR_IO;
        return (false);
    }
    return (true);
}

static bool api_https_streaming_flush_buffer(ft_string &streaming_body_buffer,
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

static bool api_https_receive_response(SSL *ssl_session, ft_string &response,
    size_t &header_length, bool &connection_close, bool &chunked_encoding,
    bool &has_length, long long &content_length, int &error_code,
    const api_streaming_handler *streaming_handler)
{
    bool streaming_enabled;
    bool headers_complete;
    ft_string header_storage;
    ft_string streaming_body_buffer;
    size_t streaming_delivered;
    bool final_chunk_sent;
    int header_status_code;
    long long chunk_stream_remaining;
    bool chunk_stream_trailers;
    char buffer[2048];

    streaming_enabled = (streaming_handler != ft_nullptr);
    headers_complete = false;
    streaming_delivered = 0;
    final_chunk_sent = false;
    header_status_code = -1;
    chunk_stream_remaining = -1;
    chunk_stream_trailers = false;
    response.clear();
    header_length = 0;
    connection_close = false;
    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    while (true)
    {
        ssize_t bytes_received;

        bytes_received = nw_ssl_read(ssl_session, buffer,
                sizeof(buffer) - 1);
        if (bytes_received < 0)
        {
            if (ft_errno == FT_ERR_SSL_WANT_READ || ft_errno == FT_ERR_SSL_WANT_WRITE)
                continue ;
            api_request_set_ssl_error(ssl_session,
                static_cast<int>(bytes_received));
            error_code = ft_errno;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
            return (false);
        }
        if (bytes_received == 0)
        {
            if (!headers_complete)
            {
                error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
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
                        error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                        return (false);
                    }
                }
                if (chunked_encoding)
                {
                    size_t consumed_length;

                    if (!api_http_chunked_body_complete(
                            response.c_str() + header_length,
                            response.size() - header_length,
                            consumed_length))
                    {
                        error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                        return (false);
                    }
                }
                break ;
            }
            if (!api_https_streaming_flush_buffer(streaming_body_buffer,
                    has_length, content_length, streaming_delivered,
                    final_chunk_sent, chunked_encoding,
                    chunk_stream_remaining, chunk_stream_trailers,
                    streaming_handler, error_code))
                return (false);
            if (chunked_encoding)
            {
                if (!final_chunk_sent || chunk_stream_trailers
                    || chunk_stream_remaining >= 0
                    || streaming_body_buffer.size() > 0)
                {
                    error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                    return (false);
                }
                break ;
            }
            if (has_length)
            {
                if (static_cast<long long>(streaming_delivered)
                    != content_length)
                {
                    error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
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
        buffer[bytes_received] = '\0';
        if (!headers_complete)
        {
            response.append(buffer, static_cast<size_t>(bytes_received));
            if (response.get_error())
            {
                error_code = response.get_error();
                return (false);
            }
            const char *headers_start;
            const char *headers_end;

            headers_start = response.c_str();
            headers_end = ft_strstr(response.c_str(), "\r\n\r\n");
            if (!headers_end)
            {
                if (!streaming_enabled && response.size() >= sizeof(buffer))
                {
                    error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
                    return (false);
                }
                continue ;
            }
            headers_end += 2;
            header_length = static_cast<size_t>(headers_end - headers_start)
                + 2;
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
                    if (!api_https_streaming_flush_buffer(
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
            headers_complete = true;
            continue ;
        }
        if (!streaming_enabled)
        {
            response.append(buffer, static_cast<size_t>(bytes_received));
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
        streaming_body_buffer.append(buffer,
            static_cast<size_t>(bytes_received));
        if (streaming_body_buffer.get_error())
        {
            error_code = streaming_body_buffer.get_error();
            return (false);
        }
        if (!api_https_streaming_flush_buffer(streaming_body_buffer,
                has_length, content_length, streaming_delivered,
                final_chunk_sent, chunked_encoding, chunk_stream_remaining,
                chunk_stream_trailers, streaming_handler, error_code))
            return (false);
        if (final_chunk_sent)
            return (true);
    }
    if (response.size() == 0)
    {
        error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
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
        error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
        return (false);
    }
    headers_end += 2;
    header_length = static_cast<size_t>(headers_end - headers_start) + 2;
    api_http_parse_headers(headers_start, headers_end, connection_close,
        chunked_encoding, has_length, content_length);
    return (true);
}

static bool api_https_execute_streaming_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const char *ca_certificate, bool verify_peer,
    const api_streaming_handler *streaming_handler, bool &connection_close,
    int &error_code)
{
    connection_close = false;
    if (!api_https_ensure_session(connection_handle, timeout,
            ca_certificate, verify_peer, error_code))
        return (false);
    SSL *ssl_session = connection_handle.tls_session;

    ft_string request;
    ft_string body_string;

    if (!api_https_prepare_request(method, path, host_header, payload,
            headers, request, body_string, error_code))
        return (false);
    if (!api_https_send_request(ssl_session, request, error_code))
        return (false);

    ft_string response;
    size_t header_length;
    bool chunked_encoding;
    bool has_length;
    long long content_length;

    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    if (!api_https_receive_response(ssl_session, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code, streaming_handler))
        return (false);
    error_code = ER_SUCCESS;
    return (true);
}

static bool api_https_prepare_request(const char *method, const char *path,
    const char *host_header, json_group *payload, const char *headers,
    ft_string &request, ft_string &body_string, int &error_code)
{
    char *temporary_string;

    if (!method || !path || !host_header)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    request.clear();
    body_string.clear();
    request += method;
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += host_header;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (headers && headers[0])
    {
        request += "\r\n";
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

static char *api_https_execute_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, int &error_code)
{
    if (!api_https_ensure_session(connection_handle, timeout,
            ca_certificate, verify_peer, error_code))
        return (ft_nullptr);
    SSL *ssl_session = connection_handle.tls_session;

    ft_string request;
    ft_string body_string;

    if (!api_https_prepare_request(method, path, host_header, payload,
            headers, request, body_string, error_code))
        return (ft_nullptr);
    if (!api_https_send_request(ssl_session, request, error_code))
        return (ft_nullptr);

    ft_string response;
    size_t header_length;
    bool connection_close;
    bool chunked_encoding;
    bool has_length;
    long long content_length;

    if (!api_https_receive_response(ssl_session, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code, ft_nullptr))
        return (ft_nullptr);
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

char *api_https_execute_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
    const api_retry_policy *retry_policy, bool &used_http2, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    bool http2_used_local;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    http2_used_local = false;
    while (attempt_index < max_attempts)
    {
        bool socket_ready;
        bool should_retry;

        socket_ready = api_https_prepare_socket(connection_handle, host, port,
                timeout, security_identity, error_code);
        if (socket_ready)
        {
            http2_used_local = false;
            char *result_body;

            result_body = api_https_execute_http2_once(connection_handle, method,
                    path, host_header, payload, headers, status, timeout,
                    ca_certificate, verify_peer, http2_used_local, error_code);
            if (result_body)
            {
                used_http2 = http2_used_local;
                return (result_body);
            }
        }
        should_retry = api_https_should_retry(error_code);
        if (!should_retry)
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
    used_http2 = false;
    if (error_code == ER_SUCCESS)
        error_code = FT_ERR_IO;
    return (ft_nullptr);
}

char *api_https_execute(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
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

        socket_ready = api_https_prepare_socket(connection_handle, host, port,
                timeout, security_identity, error_code);
        if (socket_ready)
        {
            char *result_body;

            result_body = api_https_execute_once(connection_handle, method, path,
                    host_header, payload, headers, status, timeout,
                    ca_certificate, verify_peer, error_code);
            if (result_body)
                return (result_body);
        }
        should_retry = api_https_should_retry(error_code);
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

bool api_https_execute_streaming(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
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

        socket_ready = api_https_prepare_socket(connection_handle, host, port,
                timeout, security_identity, error_code);
        if (socket_ready)
        {
            bool connection_close;
            bool success;

            connection_close = false;
            success = api_https_execute_streaming_once(connection_handle,
                    method, path, host_header, payload, headers, timeout,
                    ca_certificate, verify_peer, streaming_handler,
                    connection_close, error_code);
            if (success)
            {
                if (connection_close)
                    api_connection_pool_disable_store(connection_handle);
                return (true);
            }
        }
        should_retry = api_https_should_retry(error_code);
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

bool api_https_execute_http2_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *ca_certificate,
    bool verify_peer, const char *host, uint16_t port,
    const char *security_identity, const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &used_http2,
    int &error_code)
{
    used_http2 = false;
    return (api_https_execute_streaming(connection_handle, method, path,
            host_header, payload, headers, timeout, ca_certificate,
            verify_peer, host, port, security_identity, retry_policy,
            streaming_handler, error_code));
}

static char *api_https_execute_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, bool &used_http2,
    int &error_code)
{
    ft_vector<http2_header_field> header_fields;
    http2_header_field field_entry;
    ft_string compressed_headers;
    http2_frame headers_frame;
    ft_string encoded_frame;
    http2_stream_manager stream_manager;
    char *http_response;

    used_http2 = false;
    error_code = ER_SUCCESS;
    if (!method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    field_entry.name = ":method";
    field_entry.value = method;
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":path";
    field_entry.value = path;
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":scheme";
    field_entry.value = "https";
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":authority";
    if (host_header)
        field_entry.value = host_header;
    else
        field_entry.value = "";
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    if (headers && headers[0])
    {
        const char *header_cursor;
        ft_string header_name;
        ft_string header_value;

        header_cursor = headers;
        while (*header_cursor != '\0')
        {
            size_t index;

            header_name.clear();
            header_value.clear();
            index = 0;
            while (header_cursor[index] && header_cursor[index] != ':' && header_cursor[index] != '\r')
            {
                header_name.append(header_cursor[index]);
                if (header_name.get_error() != ER_SUCCESS)
                {
                    error_code = header_name.get_error();
                    return (ft_nullptr);
                }
                index++;
            }
            while (header_cursor[index] == ':' || header_cursor[index] == ' ')
                index++;
            while (header_cursor[index] && header_cursor[index] != '\r' && header_cursor[index] != '\n')
            {
                header_value.append(header_cursor[index]);
                if (header_value.get_error() != ER_SUCCESS)
                {
                    error_code = header_value.get_error();
                    return (ft_nullptr);
                }
                index++;
            }
            if (header_name.size() > 0)
            {
                field_entry.name = header_name;
                field_entry.value = header_value;
                header_fields.push_back(field_entry);
                if (header_fields.get_error() != ER_SUCCESS)
                {
                    error_code = header_fields.get_error();
                    return (ft_nullptr);
                }
            }
            while (header_cursor[index] == '\r' || header_cursor[index] == '\n')
                index++;
            header_cursor += index;
        }
    }
    if (!http2_compress_headers(header_fields, compressed_headers, error_code))
    {
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    headers_frame.type = 0x1;
    headers_frame.flags = 0x4;
    headers_frame.stream_id = 1;
    headers_frame.payload = compressed_headers;
    if (headers_frame.payload.get_error() != ER_SUCCESS)
    {
        error_code = headers_frame.payload.get_error();
        return (ft_nullptr);
    }
    if (!http2_encode_frame(headers_frame, encoded_frame, error_code))
    {
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    if (!stream_manager.open_stream(1))
    {
        error_code = stream_manager.get_error();
        return (ft_nullptr);
    }
    if (connection_handle.tls_session)
    {
        const unsigned char *protocol;
        unsigned int protocol_length;

        protocol = ft_nullptr;
        protocol_length = 0;
        SSL_get0_alpn_selected(connection_handle.tls_session, &protocol,
            &protocol_length);
        if (protocol && protocol_length == 2)
        {
            if (protocol[0] == 'h' && protocol[1] == '2')
                used_http2 = true;
        }
    }
    http_response = api_https_execute_once(connection_handle, method, path,
            host_header, payload, headers, status, timeout,
            ca_certificate, verify_peer, error_code);
    if (!http_response)
        return (ft_nullptr);
    size_t body_length;

    body_length = ft_strlen(http_response);
    if (!stream_manager.append_data(1, http_response, body_length))
    {
        error_code = stream_manager.get_error();
        cma_free(http_response);
        return (ft_nullptr);
    }
    if (!stream_manager.close_stream(1))
    {
        error_code = stream_manager.get_error();
        cma_free(http_response);
        return (ft_nullptr);
    }
    error_code = ER_SUCCESS;
    return (http_response);
}
