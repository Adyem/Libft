#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
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
#include <climits>

static void api_request_set_resolve_error(int resolver_status)
{
#ifdef EAI_BADFLAGS
    if (resolver_status == EAI_BADFLAGS)
    {
        ft_errno = SOCKET_RESOLVE_BAD_FLAGS;
        return ;
    }
#endif
#ifdef EAI_AGAIN
    if (resolver_status == EAI_AGAIN)
    {
        ft_errno = SOCKET_RESOLVE_AGAIN;
        return ;
    }
#endif
#ifdef EAI_FAIL
    if (resolver_status == EAI_FAIL)
    {
        ft_errno = SOCKET_RESOLVE_FAIL;
        return ;
    }
#endif
#ifdef EAI_FAMILY
    if (resolver_status == EAI_FAMILY)
    {
        ft_errno = SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_ADDRFAMILY
    if (resolver_status == EAI_ADDRFAMILY)
    {
        ft_errno = SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_SOCKTYPE
    if (resolver_status == EAI_SOCKTYPE)
    {
        ft_errno = SOCKET_RESOLVE_SOCKTYPE;
        return ;
    }
#endif
#ifdef EAI_SERVICE
    if (resolver_status == EAI_SERVICE)
    {
        ft_errno = SOCKET_RESOLVE_SERVICE;
        return ;
    }
#endif
#ifdef EAI_MEMORY
    if (resolver_status == EAI_MEMORY)
    {
        ft_errno = SOCKET_RESOLVE_MEMORY;
        return ;
    }
#endif
#ifdef EAI_NONAME
    if (resolver_status == EAI_NONAME)
    {
        ft_errno = SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_NODATA
    if (resolver_status == EAI_NODATA)
    {
        ft_errno = SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_OVERFLOW
    if (resolver_status == EAI_OVERFLOW)
    {
        ft_errno = SOCKET_RESOLVE_OVERFLOW;
        return ;
    }
#endif
#ifdef EAI_SYSTEM
    if (resolver_status == EAI_SYSTEM)
    {
#ifdef _WIN32
        ft_errno = WSAGetLastError() + ERRNO_OFFSET;
#else
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = SOCKET_RESOLVE_FAIL;
#endif
        return ;
    }
#endif
    ft_errno = SOCKET_RESOLVE_FAILED;
    return ;
}

static void api_request_set_ssl_error(SSL *ssl_session, int operation_result)
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
        int ssl_error = SSL_get_error(ssl_session, operation_result);
        if (ssl_error == SSL_ERROR_SYSCALL)
        {
#ifdef _WIN32
            int last_error = WSAGetLastError();
            if (last_error != 0)
            {
                ft_errno = last_error + ERRNO_OFFSET;
                return ;
            }
#endif
            if (errno != 0)
            {
                ft_errno = errno + ERRNO_OFFSET;
                return ;
            }
            ft_errno = SSL_ERROR_SYSCALL + ERRNO_OFFSET;
            return ;
        }
        ft_errno = ssl_error;
        return ;
    }
    ft_errno = FT_EIO;
    return ;
}

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

char *api_request_https(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer)
{
    if (ft_log_get_api_logging())
    {
        const char *log_ip = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (ip)
            log_ip = ip;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_https %s:%u %s %s",
            log_ip, port, log_method, log_path);
    }
    int error_code = ER_SUCCESS;
    struct api_request_errno_guard
    {
        int *code;
        api_request_errno_guard(int *value)
            : code(value)
        {
            return ;
        }
        ~api_request_errno_guard()
        {
            ft_errno = *code;
            return ;
        }
    } guard(&error_code);

    SocketConfig config;
    config._type = SocketType::CLIENT;
    config._ip = ip;
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_socket socket_wrapper(config);
    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (ft_nullptr);
    }
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        return (ft_nullptr);
    }
    SSL_CTX *context = ft_nullptr;
    SSL *ssl_session = ft_nullptr;
    char *result = ft_nullptr;
    ft_string request;
    ft_string body_string;
    ft_string response;
    const char *body = ft_nullptr;
    int ssl_connect_result;

    context = SSL_CTX_new(TLS_client_method());
    if (!context)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        goto cleanup;
    }
    if (verify_peer)
    {
        if (ca_certificate)
        {
            if (SSL_CTX_load_verify_locations(context, ca_certificate, ft_nullptr) != 1)
            {
                api_request_set_ssl_error(ft_nullptr, 0);
                error_code = ft_errno;
                goto cleanup;
            }
        }
        else
        {
            if (SSL_CTX_set_default_verify_paths(context) != 1)
            {
                api_request_set_ssl_error(ft_nullptr, 0);
                error_code = ft_errno;
                goto cleanup;
            }
        }
        SSL_CTX_set_verify(context, SSL_VERIFY_PEER, ft_nullptr);
    }
    else
        SSL_CTX_set_verify(context, SSL_VERIFY_NONE, ft_nullptr);
    ssl_session = SSL_new(context);
    if (!ssl_session)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        error_code = ft_errno;
        goto cleanup;
    }
    if (SSL_set_fd(ssl_session, socket_wrapper.get_fd()) != 1)
    {
        api_request_set_ssl_error(ssl_session, 0);
        error_code = ft_errno;
        goto cleanup;
    }
    ssl_connect_result = SSL_connect(ssl_session);
    if (ssl_connect_result <= 0)
    {
        api_request_set_ssl_error(ssl_session, ssl_connect_result);
        error_code = ft_errno;
        goto cleanup;
    }

    request = method;
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += ip;
    if (headers && headers[0])
    {
        request += "\r\n";
        request += headers;
    }
    if (payload)
    {
        char *temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            if (ft_errno == ER_SUCCESS)
                error_code = FT_EALLOC;
            else
                error_code = ft_errno;
            goto cleanup;
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
        {
            error_code = FT_EALLOC;
            goto cleanup;
        }
        request += "\r\nContent-Length: ";
        request += length_string;
        cma_free(length_string);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(ssl_session, request.c_str(), request.size()) < 0)
    {
        if (ft_errno == ER_SUCCESS)
        {
            api_request_set_ssl_error(ssl_session, -1);
            error_code = ft_errno;
        }
        else
            error_code = ft_errno;
        goto cleanup;
    }

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
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_EIO;
        else
            error_code = ft_errno;
        goto cleanup;
    }
    body += 4;
    result = cma_strdup(body);
    if (!result)
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_EALLOC;
        else
            error_code = ft_errno;
        goto cleanup;
    }
    error_code = ER_SUCCESS;

cleanup:
    if (ssl_session)
    {
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
    }
    if (context)
        SSL_CTX_free(context);
    return (result);
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
    int resolver_status;
    int ssl_handshake_result;

    if (!host || !method || !path)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        return (ft_nullptr);
    }

    context = SSL_CTX_new(TLS_client_method());
    if (!context)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_string[6];
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    resolver_status = getaddrinfo(host, port_string, &hints, &address_results);
    if (resolver_status != 0)
    {
        api_request_set_resolve_error(resolver_status);
        goto cleanup;
    }

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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        goto cleanup;
    }

    ssl_session = SSL_new(context);
    if (!ssl_session)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }
    if (SSL_set_fd(ssl_session, socket_fd) != 1)
    {
        api_request_set_ssl_error(ssl_session, 0);
        goto cleanup;
    }
    ssl_handshake_result = SSL_connect(ssl_session);
    if (ssl_handshake_result <= 0)
    {
        api_request_set_ssl_error(ssl_session, ssl_handshake_result);
        goto cleanup;
    }

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
        {
            if (ft_errno == ER_SUCCESS)
                ft_errno = FT_EALLOC;
            goto cleanup;
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
        {
            ft_errno = FT_EALLOC;
            goto cleanup;
        }
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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_EIO;
        goto cleanup;
    }
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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_EIO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (result)
        ft_errno = ER_SUCCESS;
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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_EIO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (result)
        ft_errno = ER_SUCCESS;
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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_EIO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (result)
        ft_errno = ER_SUCCESS;
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
    int async_resolver_status;

    if (!data || !data->host || !data->method || !data->path)
    {
        ft_errno = FT_EINVAL;
        goto cleanup;
    }
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }
    context = SSL_CTX_new(TLS_client_method());
    if (!context)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", data->port);
    async_resolver_status = getaddrinfo(data->host, port_string, &hints, &address_results);
    if (async_resolver_status != 0)
    {
        api_request_set_resolve_error(async_resolver_status);
        goto cleanup;
    }
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
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        goto cleanup;
    }

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
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        goto cleanup;
    }

    ssl_session = SSL_new(context);
    if (!ssl_session)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }
    if (SSL_set_fd(ssl_session, socket_fd) != 1)
    {
        api_request_set_ssl_error(ssl_session, 0);
        goto cleanup;
    }

    ssl_ret = SSL_connect(ssl_session);
    while (ssl_ret <= 0)
    {
        int ssl_err = SSL_get_error(ssl_session, ssl_ret);
        if (ssl_err == SSL_ERROR_WANT_READ)
        {
            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            if (select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv) <= 0)
            {
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
                goto cleanup;
            }
        }
        else if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
            {
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
                goto cleanup;
            }
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
        {
            if (ft_errno == ER_SUCCESS)
                ft_errno = FT_EALLOC;
            goto cleanup;
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
        {
            ft_errno = FT_EALLOC;
            goto cleanup;
        }
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
                {
                    if (errno != 0)
                        ft_errno = errno + ERRNO_OFFSET;
                    else if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
                    goto cleanup;
                }
                continue;
            }
            else if (ssl_err == SSL_ERROR_WANT_WRITE)
            {
                FD_ZERO(&write_set);
                FD_SET(socket_fd, &write_set);
                if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
                {
                    if (errno != 0)
                        ft_errno = errno + ERRNO_OFFSET;
                    else if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
                    goto cleanup;
                }
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
            {
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_RECEIVE_FAILED;
                break;
            }
            continue;
        }
        if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
            {
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_RECEIVE_FAILED;
                break;
            }
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
            if (!result_body && ft_errno == ER_SUCCESS)
                ft_errno = FT_EALLOC;
            if (result_body)
                ft_errno = ER_SUCCESS;
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
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    api_tls_async_request *data = static_cast<api_tls_async_request*>(cma_malloc(sizeof(api_tls_async_request)));
    if (!data)
    {
        ft_errno = FT_EALLOC;
        return (false);
    }
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
        ft_errno = FT_EALLOC;
        return (false);
    }
    ft_thread thread_worker(api_tls_async_worker, data);
    thread_worker.detach();
    ft_errno = ER_SUCCESS;
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
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    data = static_cast<api_json_async_data*>(cma_malloc(sizeof(api_json_async_data)));
    if (!data)
    {
        ft_errno = FT_EALLOC;
        return (false);
    }
    data->callback = callback;
    data->user_data = user_data;
    if (!api_request_string_tls_async(host, port, method, path, api_json_async_wrapper,
            data, payload, headers, timeout))
    {
        cma_free(data);
        ft_errno = ft_errno == ER_SUCCESS ? FT_EALLOC : ft_errno;
        return (false);
    }
    ft_errno = ER_SUCCESS;
    return (true);
}
