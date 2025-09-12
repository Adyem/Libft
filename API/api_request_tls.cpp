#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/thread.hpp"
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
# include <fcntl.h>
# include <sys/select.h>
#endif
#include <openssl/err.h>

static ssize_t ssl_send_all(SSL *ssl, const void *data, size_t size)
{
    size_t total = 0;
    const char *ptr = static_cast<const char*>(data);
    while (total < size)
    {
        ssize_t sent = nw_ssl_write(ssl, ptr + total, size - total);
        if (sent <= 0)
            return (-1);
        total += sent;
    }
    return (static_cast<ssize_t>(total));
}

char *api_request_string_tls(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    if (ft_log_get_api_logging())
    {
        const char *log_host = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (host)
            log_host = host;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_string_tls %s:%u %s %s",
            log_host, port, log_method, log_path);
    }
    SSL_CTX *context = ft_nullptr;
    SSL *ssl_session = ft_nullptr;
    int socket_fd = -1;
    char *result = ft_nullptr;
    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_string request;
    ft_string body_string;
    ft_string response;
    const char *body = ft_nullptr;

    if (!host || !method || !path)
        return (ft_nullptr);
    if (!OPENSSL_init_ssl(0, ft_nullptr))
        return (ft_nullptr);

    context = SSL_CTX_new(TLS_client_method());
    if (!context)
        goto cleanup;

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_string[6];
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host, port_string, &hints, &address_results) != 0)
        goto cleanup;

    address_info = address_results;
    while (address_info != ft_nullptr)
    {
        socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (socket_fd >= 0)
        {
            if (timeout > 0)
            {
                struct timeval tv;
                tv.tv_sec = timeout / 1000;
                tv.tv_usec = (timeout % 1000) * 1000;
                setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
            }
            if (nw_connect(socket_fd, address_info->ai_addr, static_cast<socklen_t>(address_info->ai_addrlen)) == 0)
                break;
            FT_CLOSE_SOCKET(socket_fd);
            socket_fd = -1;
        }
        address_info = address_info->ai_next;
    }
    freeaddrinfo(address_results);
    address_results = ft_nullptr;
    if (socket_fd < 0)
        goto cleanup;

    ssl_session = SSL_new(context);
    if (!ssl_session)
        goto cleanup;
    if (SSL_set_fd(ssl_session, socket_fd) != 1)
        goto cleanup;
    if (SSL_connect(ssl_session) <= 0)
        goto cleanup;

    request = method;
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += host;
    if (headers && headers[0])
    {
        request += "\r\n";
        request += headers;
    }
    if (payload)
    {
        char *temporary_string = json_write_to_string(payload);
        if (!temporary_string)
            goto cleanup;
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
            goto cleanup;
        request += "\r\nContent-Length: ";
        request += length_string;
        cma_free(length_string);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(ssl_session, request.c_str(), request.size()) < 0)
        goto cleanup;

    char buffer[1024];
    ssize_t bytes_received;
    while ((bytes_received = nw_ssl_read(ssl_session, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    body = ft_strstr(response.c_str(), "\r\n\r\n");
    if (!body)
        goto cleanup;
    body += 4;
    result = cma_strdup(body);

cleanup:
    if (ssl_session)
    {
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
    }
    if (socket_fd >= 0)
        FT_CLOSE_SOCKET(socket_fd);
    if (context)
        SSL_CTX_free(context);
    if (address_results)
        freeaddrinfo(address_results);
    return (result);
}

json_group *api_request_json_tls(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string_tls(host, port, method, path, payload,
                                        headers, status, timeout);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

char *api_request_string_tls_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!token)
        return (api_request_string_tls(host, port, method, path, payload,
                                       headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Bearer ";
    header_string += token;
    return (api_request_string_tls(host, port, method, path, payload,
                                   header_string.c_str(), status, timeout));
}

json_group *api_request_json_tls_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_tls_bearer(host, port, method, path, token,
                                               payload, headers, status, timeout);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

char *api_request_string_tls_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!credentials)
        return (api_request_string_tls(host, port, method, path, payload,
                                       headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Basic ";
    header_string += credentials;
    return (api_request_string_tls(host, port, method, path, payload,
                                   header_string.c_str(), status, timeout));
}

json_group *api_request_json_tls_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_tls_basic(host, port, method, path,
                                              credentials, payload, headers,
                                              status, timeout);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

struct api_tls_async_request
{
    char *host;
    uint16_t port;
    char *method;
    char *path;
    json_group *payload;
    char *headers;
    int timeout;
    api_callback callback;
    void *user_data;
};

static void api_tls_async_worker(api_tls_async_request *data)
{
    SSL_CTX *context = ft_nullptr;
    SSL *ssl_session = ft_nullptr;
    int socket_fd = -1;
    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_string request;
    ft_string body_string;
    ft_string response;
    char port_string[6];
    char *result_body = ft_nullptr;
    int status = -1;
    char buffer[1024];
    fd_set read_set;
    fd_set write_set;
    struct timeval tv;
    size_t total_sent;
    int ssl_ret;

    if (!data || !data->host || !data->method || !data->path)
        goto cleanup;
    if (!OPENSSL_init_ssl(0, ft_nullptr))
        goto cleanup;
    context = SSL_CTX_new(TLS_client_method());
    if (!context)
        goto cleanup;

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", data->port);
    if (getaddrinfo(data->host, port_string, &hints, &address_results) != 0)
        goto cleanup;
    address_info = address_results;
    while (address_info != ft_nullptr && socket_fd < 0)
    {
        socket_fd = nw_socket(address_info->ai_family,
                               address_info->ai_socktype,
                               address_info->ai_protocol);
        if (socket_fd < 0)
            address_info = address_info->ai_next;
    }
    if (socket_fd < 0)
        goto cleanup;

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(socket_fd, FIONBIO, &mode);
#else
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
#endif

    if (nw_connect(socket_fd, address_info->ai_addr,
                    static_cast<socklen_t>(address_info->ai_addrlen)) < 0)
    {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err != WSAEINPROGRESS && err != WSAEWOULDBLOCK)
            goto cleanup;
#else
        if (errno != EINPROGRESS)
            goto cleanup;
#endif
    }

    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    tv.tv_sec = data->timeout / 1000;
    tv.tv_usec = (data->timeout % 1000) * 1000;
    if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
        goto cleanup;

    ssl_session = SSL_new(context);
    if (!ssl_session)
        goto cleanup;
    if (SSL_set_fd(ssl_session, socket_fd) != 1)
        goto cleanup;

    ssl_ret = SSL_connect(ssl_session);
    while (ssl_ret <= 0)
    {
        int ssl_err = SSL_get_error(ssl_session, ssl_ret);
        if (ssl_err == SSL_ERROR_WANT_READ)
        {
            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            if (select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv) <= 0)
                goto cleanup;
        }
        else if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
                goto cleanup;
        }
        else
            goto cleanup;
        ssl_ret = SSL_connect(ssl_session);
    }

    request = data->method;
    request += " ";
    request += data->path;
    request += " HTTP/1.1\r\nHost: ";
    request += data->host;
    if (data->headers && data->headers[0])
    {
        request += "\r\n";
        request += data->headers;
    }
    if (data->payload)
    {
        char *temporary_string = json_write_to_string(data->payload);
        if (!temporary_string)
            goto cleanup;
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
            goto cleanup;
        request += "\r\nContent-Length: ";
        request += length_string;
        cma_free(length_string);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (data->payload)
        request += body_string.c_str();

    total_sent = 0;
    while (total_sent < request.size())
    {
        int write_ret = SSL_write(ssl_session, request.c_str() + total_sent,
                                  static_cast<int>(request.size() - total_sent));
        if (write_ret <= 0)
        {
            int ssl_err = SSL_get_error(ssl_session, write_ret);
            if (ssl_err == SSL_ERROR_WANT_READ)
            {
                FD_ZERO(&read_set);
                FD_SET(socket_fd, &read_set);
                if (select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv) <= 0)
                    goto cleanup;
                continue;
            }
            else if (ssl_err == SSL_ERROR_WANT_WRITE)
            {
                FD_ZERO(&write_set);
                FD_SET(socket_fd, &write_set);
                if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
                    goto cleanup;
                continue;
            }
            goto cleanup;
        }
        total_sent += static_cast<size_t>(write_ret);
    }

    while (true)
    {
        int read_ret = SSL_read(ssl_session, buffer, sizeof(buffer) - 1);
        if (read_ret > 0)
        {
            buffer[read_ret] = '\0';
            response += buffer;
            continue;
        }
        int ssl_err = SSL_get_error(ssl_session, read_ret);
        if (ssl_err == SSL_ERROR_WANT_READ)
        {
            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            if (select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv) <= 0)
                break;
            continue;
        }
        if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
                break;
            continue;
        }
        break;
    }

    if (response.size() > 0)
    {
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            status = ft_atoi(space + 1);
        const char *body = ft_strstr(response.c_str(), "\r\n\r\n");
        if (body)
        {
            body += 4;
            result_body = cma_strdup(body);
        }
    }

cleanup:
    if (data->callback)
        data->callback(result_body, status, data->user_data);
    if (ssl_session)
    {
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
    }
    if (socket_fd >= 0)
        FT_CLOSE_SOCKET(socket_fd);
    if (context)
        SSL_CTX_free(context);
    if (address_results)
        freeaddrinfo(address_results);
    if (data->host)
        cma_free(data->host);
    if (data->method)
        cma_free(data->method);
    if (data->path)
        cma_free(data->path);
    if (data->headers)
        cma_free(data->headers);
    cma_free(data);
    return ;
}

bool    api_request_string_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    if (!host || !method || !path || !callback)
        return (false);
    api_tls_async_request *data = static_cast<api_tls_async_request*>(cma_malloc(sizeof(api_tls_async_request)));
    if (!data)
        return (false);
    ft_bzero(data, sizeof(api_tls_async_request));
    data->host = cma_strdup(host);
    data->method = cma_strdup(method);
    data->path = cma_strdup(path);
    if (headers)
        data->headers = cma_strdup(headers);
    data->port = port;
    data->payload = payload;
    data->timeout = timeout;
    data->callback = callback;
    data->user_data = user_data;
    if (!data->host || !data->method || !data->path || (headers && !data->headers))
    {
        if (data->host)
            cma_free(data->host);
        if (data->method)
            cma_free(data->method);
        if (data->path)
            cma_free(data->path);
        if (data->headers)
            cma_free(data->headers);
        cma_free(data);
        return (false);
    }
    ft_thread thread_worker(api_tls_async_worker, data);
    thread_worker.detach();
    return (true);
}

struct api_json_async_data
{
    api_json_callback callback;
    void *user_data;
};

static void api_json_async_wrapper(char *body, int status, void *user_data)
{
    api_json_async_data *data;
    json_group *json_body;

    data = static_cast<api_json_async_data*>(user_data);
    json_body = ft_nullptr;
    if (body)
    {
        json_body = json_read_from_string(body);
        cma_free(body);
    }
    if (data && data->callback)
        data->callback(json_body, status, data->user_data);
    if (data)
        cma_free(data);
    return ;
}

bool    api_request_json_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    api_json_async_data *data;

    if (!host || !method || !path || !callback)
        return (false);
    data = static_cast<api_json_async_data*>(cma_malloc(sizeof(api_json_async_data)));
    if (!data)
        return (false);
    data->callback = callback;
    data->user_data = user_data;
    if (!api_request_string_tls_async(host, port, method, path, api_json_async_wrapper,
            data, payload, headers, timeout))
    {
        cma_free(data);
        return (false);
    }
    return (true);
}
