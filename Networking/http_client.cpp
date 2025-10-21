#include "http_client.hpp"
#include "http2_client.hpp"
#include "socket_class.hpp"
#include "networking.hpp"
#include "ssl_wrapper.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <openssl/x509v3.h>
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include "../Observability/observability_networking_metrics.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

struct http_stream_state
{
    ft_string       header_buffer;
    ft_string       headers;
    int             status_code;
    bool            headers_ready;
    http_response_handler handler;
};

struct http_buffer_adapter_state
{
    ft_string   *response;
    bool        header_appended;
    size_t      body_bytes;
    int         status_code;
};

static http_buffer_adapter_state g_http_buffer_adapter_state = { NULL, false, 0, 0 };

static int http_client_wait_for_socket_ready(int socket_fd, bool wait_for_write)
{
    int poll_descriptor;
    int poll_result;

    if (socket_fd < 0)
    {
        ft_errno = FT_ERR_SOCKET_SEND_FAILED;
        return (-1);
    }
    poll_descriptor = socket_fd;
    if (wait_for_write != false)
        poll_result = nw_poll(NULL, 0, &poll_descriptor, 1, 1000);
    else
        poll_result = nw_poll(&poll_descriptor, 1, NULL, 0, 1000);
    if (poll_result < 0)
    {
#ifdef _WIN32
        ft_errno = ft_map_system_error(WSAGetLastError());
#else
        ft_errno = ft_map_system_error(errno);
#endif
        return (-1);
    }
    if (poll_result == 0)
        return (1);
    return (0);
}

static int http_client_initialize_ssl(int socket_fd, const char *host, SSL_CTX **ssl_context, SSL **ssl_connection)
{
    SSL_CTX *local_context;
    SSL *local_connection;

    if (ssl_context == NULL || ssl_connection == NULL)
    {
        ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    *ssl_context = NULL;
    *ssl_connection = NULL;
    SSL_library_init();
    local_context = SSL_CTX_new(TLS_client_method());
    if (local_context == NULL)
    {
        ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    SSL_CTX_set_verify(local_context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(local_context) != 1)
    {
        SSL_CTX_free(local_context);
        ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    local_connection = SSL_new(local_context);
    if (local_connection == NULL)
    {
        SSL_CTX_free(local_context);
        ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    bool selected_http2;
    int alpn_error;

    if (!http2_select_alpn_protocol(local_connection, selected_http2, alpn_error))
        ft_errno = ER_SUCCESS;
    (void)selected_http2;
    (void)alpn_error;
    if (host != NULL && host[0] != '\0')
    {
        long control_result;

        control_result = SSL_ctrl(local_connection, SSL_CTRL_SET_TLSEXT_HOSTNAME,
            TLSEXT_NAMETYPE_host_name, const_cast<char *>(host));
        if (control_result != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        X509_VERIFY_PARAM *verify_params;

        verify_params = SSL_get0_param(local_connection);
        if (verify_params == NULL)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            return (-1);
        }
        X509_VERIFY_PARAM_set_hostflags(verify_params, 0);
        if (X509_VERIFY_PARAM_set1_host(verify_params, host, 0) != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            return (-1);
        }
#endif
    }
    if (SSL_set_fd(local_connection, socket_fd) != 1)
    {
        SSL_free(local_connection);
        SSL_CTX_free(local_context);
        ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    *ssl_context = local_context;
    *ssl_connection = local_connection;
    return (0);
}

int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;

    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_send(socket_fd, buffer + total_sent, length - total_sent, 0);
        if (send_result < 0)
        {
            int wait_result;

#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEINTR)
                continue ;
            if (last_error == WSAEWOULDBLOCK)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            ft_errno = ft_map_system_error(last_error);
#else
            if (errno == EINTR)
                continue ;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            ft_errno = ft_map_system_error(errno);
#endif
            return (-1);
        }
        if (send_result == 0)
        {
            ft_errno = FT_ERR_SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_socket_after_send(socket_fd) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;
    int socket_fd;

    socket_fd = -1;
    if (ssl_connection != NULL)
        socket_fd = SSL_get_fd(ssl_connection);
    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_ssl_write(ssl_connection, buffer + total_sent, length - total_sent);
        if (send_result <= 0)
        {
            int ssl_error;
            int wait_result;

            ssl_error = SSL_get_error(ssl_connection, static_cast<int>(send_result));
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
            {
                bool wait_for_write;

                wait_for_write = (ssl_error == SSL_ERROR_WANT_WRITE);
                wait_result = http_client_wait_for_socket_ready(socket_fd, wait_for_write);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            if (ssl_error == SSL_ERROR_SYSCALL)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (last_error == WSAEINTR)
                    continue ;
                if (last_error == WSAEWOULDBLOCK)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error != 0)
                {
                    ft_errno = ft_map_system_error(last_error);
                    return (-1);
                }
#else
                if (errno == EINTR)
                    continue ;
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (errno != 0)
                {
                    ft_errno = ft_map_system_error(errno);
                    return (-1);
                }
#endif
            }
            ft_errno = FT_ERR_SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_ssl_after_send(ssl_connection) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

static void http_client_stream_state_init(http_stream_state &state, http_response_handler handler)
{
    state.header_buffer.clear();
    state.headers.clear();
    state.status_code = 0;
    state.headers_ready = false;
    state.handler = handler;
    return ;
}

static int http_client_parse_status(const ft_string &headers)
{
    const char  *header_cstr;
    size_t      index;

    header_cstr = headers.c_str();
    index = 0;
    while (header_cstr[index] != '\0' && header_cstr[index] != ' ')
        index++;
    while (header_cstr[index] == ' ')
        index++;
    if (header_cstr[index] == '\0')
        return (0);
    return (ft_atoi(header_cstr + index));
}

static int http_client_stream_handle_header(http_stream_state &state)
{
    const char  *terminator;
    size_t      header_length;
    size_t      body_length;

    terminator = ft_strstr(state.header_buffer.c_str(), "\r\n\r\n");
    if (terminator == NULL)
        return (0);
    header_length = static_cast<size_t>(terminator - state.header_buffer.c_str()) + 4;
    state.headers = state.header_buffer;
    if (state.headers.size() > header_length)
        state.headers.erase(header_length, state.headers.size() - header_length);
    state.status_code = http_client_parse_status(state.headers);
    body_length = state.header_buffer.size() - header_length;
    if (body_length > 0 && state.handler != NULL)
    {
        const char *body_ptr;

        body_ptr = state.header_buffer.c_str() + header_length;
        state.handler(state.status_code, state.headers, body_ptr, body_length, false);
    }
    state.header_buffer.clear();
    state.headers_ready = true;
    return (1);
}

static int http_client_receive_stream(int socket_fd, SSL *ssl_connection, bool use_ssl,
    http_response_handler handler)
{
    char                buffer[1024];
    ssize_t             bytes_received;
    http_stream_state   state;

    if (handler == NULL)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    http_client_stream_state_init(state, handler);
    while (1)
    {
        if (use_ssl != false)
            bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        else
            bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            if (state.headers_ready == false)
            {
                state.header_buffer.append(buffer);
                if (http_client_stream_handle_header(state) != 0)
                    continue ;
            }
            else
                state.handler(state.status_code, state.headers, buffer,
                    static_cast<size_t>(bytes_received), false);
        }
        else
        {
            if (use_ssl != false)
            {
                if (bytes_received == 0)
                {
                    if (ft_errno == FT_ERR_SSL_WANT_READ || ft_errno == FT_ERR_SSL_WANT_WRITE)
                    {
                        int wait_result;
                        bool wait_for_write;

                        wait_for_write = (ft_errno == FT_ERR_SSL_WANT_WRITE);
                        wait_result = http_client_wait_for_socket_ready(socket_fd, wait_for_write);
                        if (wait_result < 0)
                            return (-1);
                        continue ;
                    }
                    if (ft_errno == FT_ERR_SSL_ZERO_RETURN)
                        break;
                    if (ft_errno == FT_ERR_SSL_SYSCALL_ERROR)
                    {
#ifdef _WIN32
                        int last_error;

                        last_error = WSAGetLastError();
                        if (last_error == WSAEINTR)
                            continue ;
                        if (last_error == WSAEWOULDBLOCK)
                        {
                            int wait_result;

                            wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                            if (wait_result < 0)
                                return (-1);
                            continue ;
                        }
                        if (last_error != 0)
                            ft_errno = ft_map_system_error(last_error);
                        else
                            ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#else
                        int last_error;

                        last_error = errno;
                        if (last_error == EINTR)
                            continue ;
                        if (last_error == EWOULDBLOCK || last_error == EAGAIN)
                        {
                            int wait_result;

                            wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                            if (wait_result < 0)
                                return (-1);
                            continue ;
                        }
                        if (last_error != 0)
                            ft_errno = ft_map_system_error(last_error);
                        else
                            ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#endif
                        return (-1);
                    }
                    ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
                    return (-1);
                }
                if (ft_errno == FT_ERR_SSL_SYSCALL_ERROR)
                {
#ifdef _WIN32
                    int last_error;

                    last_error = WSAGetLastError();
                    if (last_error == WSAEINTR)
                        continue ;
                    if (last_error == WSAEWOULDBLOCK)
                    {
                        int wait_result;

                        wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                        if (wait_result < 0)
                            return (-1);
                        continue ;
                    }
                    if (last_error != 0)
                        ft_errno = ft_map_system_error(last_error);
                    else
                        ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#else
                    int last_error;

                    last_error = errno;
                    if (last_error == EINTR)
                        continue ;
                    if (last_error == EWOULDBLOCK || last_error == EAGAIN)
                    {
                        int wait_result;

                        wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                        if (wait_result < 0)
                            return (-1);
                        continue ;
                    }
                    if (last_error != 0)
                        ft_errno = ft_map_system_error(last_error);
                    else
                        ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#endif
                }
                return (-1);
            }
            if (bytes_received < 0)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (last_error == WSAEINTR)
                    continue ;
                if (last_error == WSAEWOULDBLOCK)
                {
                    int wait_result;

                    wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error == WSAECONNRESET)
                    break;
                if (last_error != 0)
                    ft_errno = ft_map_system_error(last_error);
                else
                    ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#else
                int last_error;

                last_error = errno;
                if (last_error == EINTR)
                    continue ;
                if (last_error == EWOULDBLOCK || last_error == EAGAIN)
                {
                    int wait_result;

                    wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error == ECONNRESET)
                    break;
                if (last_error != 0)
                    ft_errno = ft_map_system_error(last_error);
                else
                    ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
#endif
                return (-1);
            }
            break;
        }
    }
    if (state.headers_ready == false && state.header_buffer.empty() == false)
    {
        const char *body_ptr;

        body_ptr = state.header_buffer.c_str();
        state.handler(state.status_code, state.headers, body_ptr,
            state.header_buffer.size(), false);
    }
    state.handler(state.status_code, state.headers, "", 0, true);
    return (0);
}

static void http_client_buffering_adapter(int status_code, const ft_string &headers,
    const char *body_chunk, size_t chunk_size, bool finished)
{
    if (g_http_buffer_adapter_state.response == NULL)
        return ;
    if (status_code != 0)
        g_http_buffer_adapter_state.status_code = status_code;
    if (g_http_buffer_adapter_state.header_appended == false && headers.empty() == false)
    {
        g_http_buffer_adapter_state.response->append(headers);
        g_http_buffer_adapter_state.header_appended = true;
    }
    if (chunk_size > 0)
    {
        g_http_buffer_adapter_state.response->append(body_chunk);
        g_http_buffer_adapter_state.body_bytes += chunk_size;
    }
    if (finished != false && status_code != 0)
        g_http_buffer_adapter_state.status_code = status_code;
    return ;
}

static void http_client_reset_buffer_adapter_state(void)
{
    g_http_buffer_adapter_state.response = NULL;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    return ;
}

static int http_client_finish_with_metrics(const char *method, const char *host,
    const char *path, size_t request_bytes, int result,
    t_monotonic_time_point start_time)
{
    ft_networking_observability_sample sample;
    t_monotonic_time_point finish_time;
    long long duration_ms;
    int error_code;

    finish_time = time_monotonic_point_now();
    duration_ms = time_monotonic_point_diff_ms(start_time, finish_time);
    if (duration_ms < 0)
        duration_ms = 0;
    error_code = ER_SUCCESS;
    if (result != 0)
        error_code = ft_errno;
    sample.labels.component = "http_client";
    sample.labels.operation = method;
    sample.labels.target = host;
    sample.labels.resource = path;
    sample.duration_ms = duration_ms;
    sample.request_bytes = request_bytes;
    sample.response_bytes = g_http_buffer_adapter_state.body_bytes;
    sample.status_code = g_http_buffer_adapter_state.status_code;
    sample.error_code = error_code;
    if (error_code == ER_SUCCESS)
    {
        sample.success = true;
        sample.error_tag = "ok";
    }
    else
    {
        sample.success = false;
        sample.error_tag = ft_strerror(error_code);
    }
    observability_networking_metrics_record(sample);
    http_client_reset_buffer_adapter_state();
    return (result);
}

int http_get_stream(const char *host, const char *path, http_response_handler handler,
    bool use_ssl, const char *custom_port)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
    const char *port_string;
    int socket_fd;
    ft_string request;
    SSL_CTX *ssl_context;
    SSL *ssl_connection;
    int result;
    int resolver_status;
    int last_socket_error;

    if (handler == NULL)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    resolver_status = getaddrinfo(host, port_string, &address_hints, &address_info);
    if (resolver_status != 0)
    {
        networking_dns_set_error(resolver_status);
        return (-1);
    }
    socket_fd = -1;
    result = -1;
    last_socket_error = 0;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
            nw_close(socket_fd);
            socket_fd = -1;
        }
        else
        {
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
    {
        if (last_socket_error != 0)
            ft_errno = ft_map_system_error(last_socket_error);
        else
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (-1);
    }
    request.append("GET ");
    request.append(path);
    request.append(" HTTP/1.1\r\nHost: ");
    request.append(host);
    request.append("\r\nConnection: close\r\n\r\n");
    if (use_ssl)
    {
        if (http_client_initialize_ssl(socket_fd, host, &ssl_context, &ssl_connection) != 0)
        {
            nw_close(socket_fd);
            return (-1);
        }
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (SSL_get_verify_result(ssl_connection) != X509_V_OK)
        {
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (-1);
        }
#endif
        if (http_client_send_ssl_request(ssl_connection, request.c_str(), request.size()) != 0)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (-1);
        }
        if (http_client_receive_stream(socket_fd, ssl_connection, true, handler) != 0)
        {
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (-1);
        }
        SSL_shutdown(ssl_connection);
        SSL_free(ssl_connection);
        SSL_CTX_free(ssl_context);
    }
    else
    {
        if (http_client_send_plain_request(socket_fd, request.c_str(), request.size()) != 0)
        {
            nw_close(socket_fd);
            return (-1);
        }
        if (http_client_receive_stream(socket_fd, NULL, false, handler) != 0)
        {
            nw_close(socket_fd);
            return (-1);
        }
    }
    nw_close(socket_fd);
    ft_errno = ER_SUCCESS;
    return (0);
}

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl, const char *custom_port)
{
    t_monotonic_time_point start_time;
    int result;

    start_time = time_monotonic_point_now();
    response.clear();
    g_http_buffer_adapter_state.response = &response;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    result = http_get_stream(host, path, http_client_buffering_adapter, use_ssl, custom_port);
    return (http_client_finish_with_metrics("GET", host, path, 0, result, start_time));
}

int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl, const char *custom_port)
{
    t_monotonic_time_point start_time;
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
    const char *port_string;
    int socket_fd;
    ft_string request;
    char length_string[32];
    SSL_CTX *ssl_context;
    SSL *ssl_connection;
    int result;
    int resolver_status;
    int last_socket_error;

    start_time = time_monotonic_point_now();
    response.clear();
    g_http_buffer_adapter_state.response = &response;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    ft_memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    resolver_status = getaddrinfo(host, port_string, &address_hints, &address_info);
    if (resolver_status != 0)
    {
        networking_dns_set_error(resolver_status);
        return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
    }
    socket_fd = -1;
    result = -1;
    last_socket_error = 0;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
            nw_close(socket_fd);
            socket_fd = -1;
        }
        else
        {
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
    {
        if (last_socket_error != 0)
            ft_errno = ft_map_system_error(last_socket_error);
        else
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
        return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
    }
    std::snprintf(length_string, sizeof(length_string), "%zu", body.size());
    request.append("POST ");
    request.append(path);
    request.append(" HTTP/1.1\r\nHost: ");
    request.append(host);
    request.append("\r\nContent-Length: ");
    request.append(length_string);
    request.append("\r\nConnection: close\r\n\r\n");
    request.append(body);
    if (use_ssl)
    {
        if (http_client_initialize_ssl(socket_fd, host, &ssl_context, &ssl_connection) != 0)
        {
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (SSL_get_verify_result(ssl_connection) != X509_V_OK)
        {
            ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
#endif
        if (http_client_send_ssl_request(ssl_connection, request.c_str(), request.size()) != 0)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
        if (http_client_receive_stream(socket_fd, ssl_connection, true, http_client_buffering_adapter) != 0)
        {
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
        SSL_shutdown(ssl_connection);
        SSL_free(ssl_connection);
        SSL_CTX_free(ssl_context);
    }
    else
    {
        if (http_client_send_plain_request(socket_fd, request.c_str(), request.size()) != 0)
        {
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
        if (http_client_receive_stream(socket_fd, NULL, false, http_client_buffering_adapter) != 0)
        {
            nw_close(socket_fd);
            return (http_client_finish_with_metrics("POST", host, path, body.size(), -1, start_time));
        }
    }
    nw_close(socket_fd);
    ft_errno = ER_SUCCESS;
    return (http_client_finish_with_metrics("POST", host, path, body.size(), 0, start_time));
}

