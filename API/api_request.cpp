#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
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

char *api_request_string(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
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
        ft_log_debug("api_request_string %s:%u %s %s",
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

    ft_string request(method);
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += ip;
    if (headers && headers[0])
    {
        request += "\r\n";
        request += headers;
    }

    ft_string body_string;
    if (payload)
    {
        char *temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            if (ft_errno == ER_SUCCESS)
                error_code = FT_EALLOC;
            else
                error_code = ft_errno;
            return (ft_nullptr);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
        {
            error_code = FT_EALLOC;
            return (ft_nullptr);
        }
        request += "\r\nContent-Length: ";
        request += length_string;
        cma_free(length_string);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    size_t total_bytes_sent = 0;
    bool send_failed = false;
    int send_failure_code = SOCKET_SEND_FAILED;
    while (total_bytes_sent < request.size())
    {
        ssize_t chunk_bytes_sent;

        chunk_bytes_sent = socket_wrapper.send_data(request.c_str() + total_bytes_sent,
            request.size() - total_bytes_sent, 0);
        if (chunk_bytes_sent < 0)
        {
            send_failed = true;
            send_failure_code = socket_wrapper.get_error();
            if (send_failure_code == ER_SUCCESS)
                send_failure_code = SOCKET_SEND_FAILED;
            break;
        }
        if (chunk_bytes_sent == 0)
        {
            send_failed = true;
            send_failure_code = SOCKET_SEND_FAILED;
            break;
        }
        total_bytes_sent += static_cast<size_t>(chunk_bytes_sent);
    }

    char buffer[1024];
    ft_string response;
    ssize_t bytes_received;

    while (true)
    {
        bytes_received = socket_wrapper.receive_data(buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            response += buffer;
            continue ;
        }
        if (bytes_received == 0)
        {
            break;
        }
        int receive_error = socket_wrapper.get_error();
#ifdef ECONNRESET
        if (!response.empty() && receive_error == ECONNRESET + ERRNO_OFFSET)
        {
            break;
        }
#endif
#ifdef ECONNABORTED
        if (!response.empty() && receive_error == ECONNABORTED + ERRNO_OFFSET)
        {
            break;
        }
#endif
        error_code = receive_error;
        if (response.empty())
        {
            if (send_failed)
                error_code = send_failure_code;
            else
                error_code = SOCKET_SEND_FAILED;
            return (ft_nullptr);
        }
        if (error_code == ER_SUCCESS)
            error_code = SOCKET_RECEIVE_FAILED;
        return (ft_nullptr);
    }
    if (response.empty())
    {
        if (send_failed)
            error_code = send_failure_code;
        else
            error_code = SOCKET_SEND_FAILED;
        return (ft_nullptr);
    }
    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    const char *body = ft_strstr(response.c_str(), "\r\n\r\n");
    if (!body)
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_EIO;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    body += 4;
    char *result_body = cma_strdup(body);
    if (!result_body)
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_EALLOC;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    error_code = ER_SUCCESS;
    return (result_body);
}

json_group *api_request_json(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string(ip, port, method, path, payload,
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

char *api_request_string_host(const char *host, uint16_t port,
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
        ft_log_debug("api_request_string_host %s:%u %s %s",
            log_host, port, log_method, log_path);
    }
    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    int socket_fd = -1;
    ft_string request;
    ft_string body_string;
    ft_string response;
    char *result = ft_nullptr;
    char buffer[1024];
    ssize_t bytes_received;
    const char *body = ft_nullptr;
    char port_string[6];

    if (!host || !method || !path)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    int resolver_status = getaddrinfo(host, port_string, &hints, &address_results);
    if (resolver_status != 0)
    {
        api_request_set_resolve_error(resolver_status);
        goto cleanup;
    }

    address_info = address_results;
    while (address_info != ft_nullptr)
    {
        socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype,
                address_info->ai_protocol);
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
            if (nw_connect(socket_fd, address_info->ai_addr,
                        static_cast<socklen_t>(address_info->ai_addrlen)) == 0)
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
        ft_errno = SOCKET_CONNECT_FAILED;
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

    if (nw_send(socket_fd, request.c_str(), request.size(), 0) < 0)
    {
#ifdef _WIN32
        int send_error = WSAGetLastError();
        if (send_error != 0)
            ft_errno = send_error + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_SEND_FAILED;
#else
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_SEND_FAILED;
#endif
        goto cleanup;
    }

    while ((bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        response += buffer;
    }
    if (bytes_received < 0)
    {
#ifdef _WIN32
        int recv_error = WSAGetLastError();
        if (recv_error != 0)
            ft_errno = recv_error + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_RECEIVE_FAILED;
#else
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_RECEIVE_FAILED;
#endif
        goto cleanup;
    }
    if (response.empty())
    {
        ft_errno = SOCKET_SEND_FAILED;
        goto cleanup;
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
    if (!result && ft_errno == ER_SUCCESS)
        ft_errno = FT_EALLOC;
    if (result)
        ft_errno = ER_SUCCESS;

cleanup:
    if (socket_fd >= 0)
        FT_CLOSE_SOCKET(socket_fd);
    if (address_results)
        freeaddrinfo(address_results);
    return (result);
}

json_group *api_request_json_host(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string_host(host, port, method, path, payload,
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

char *api_request_string_bearer(const char *ip, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!token)
        return (api_request_string(ip, port, method, path, payload,
                                   headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Bearer ";
    header_string += token;
    return (api_request_string(ip, port, method, path, payload,
                               header_string.c_str(), status, timeout));
}

json_group *api_request_json_bearer(const char *ip, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_bearer(ip, port, method, path, token,
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

char *api_request_string_basic(const char *ip, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!credentials)
        return (api_request_string(ip, port, method, path, payload,
                                   headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Basic ";
    header_string += credentials;
    return (api_request_string(ip, port, method, path, payload,
                               header_string.c_str(), status, timeout));
}

json_group *api_request_json_basic(const char *ip, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_basic(ip, port, method, path, credentials,
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

char *api_request_string_host_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!token)
        return (api_request_string_host(host, port, method, path, payload,
                                        headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Bearer ";
    header_string += token;
    return (api_request_string_host(host, port, method, path, payload,
                                    header_string.c_str(), status, timeout));
}

json_group *api_request_json_host_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_host_bearer(host, port, method, path, token,
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

char *api_request_string_host_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (!credentials)
        return (api_request_string_host(host, port, method, path, payload,
                                        headers, status, timeout));
    ft_string header_string;
    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Basic ";
    header_string += credentials;
    return (api_request_string_host(host, port, method, path, payload,
                                    header_string.c_str(), status, timeout));
}

json_group *api_request_json_host_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_host_basic(host, port, method, path,
                                               credentials, payload, headers,
                                               status, timeout);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (result)
        ft_errno = ER_SUCCESS;
    return (result);
}

static bool parse_url(const char *url, bool &tls, ft_string &host,
                      uint16_t &port, ft_string &path)
{
    const char *scheme_end;
    const char *host_start;
    const char *path_start;
    const char *walker;
    const char *colon;
    ft_string       scheme;
    ft_string       hostport;
    const char      *slash;

    if (!url)
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    scheme_end = ft_strstr(url, "://");
    if (!scheme_end)
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    walker = url;
    while (walker < scheme_end)
    {
        scheme.append(*walker);
        if (scheme.get_error())
            return (false);
        walker++;
    }
    tls = (scheme == "https");
    host_start = scheme_end + 3;
    path_start = ft_strchr(host_start, '/');
    path.clear();
    if (path_start)
    {
        path.append(path_start);
        if (path.get_error())
            return (false);
    }
    else
    {
        slash = "/";
        path = slash;
        if (path.get_error())
            return (false);
    }
    if (path_start)
    {
        hostport.clear();
        walker = host_start;
        while (walker < path_start)
        {
            hostport.append(*walker);
            if (hostport.get_error())
                return (false);
            walker++;
        }
    }
    else
    {
        hostport.clear();
        hostport.append(host_start);
        if (hostport.get_error())
            return (false);
    }
    colon = ft_strchr(hostport.c_str(), ':');
    if (colon)
    {
        host.clear();
        walker = hostport.c_str();
        while (walker < colon)
        {
            host.append(*walker);
            if (host.get_error())
                return (false);
            walker++;
        }
        port = static_cast<uint16_t>(ft_atoi(colon + 1));
    }
    else
    {
        host = hostport;
        if (host.get_error())
            return (false);
        if (tls)
            port = 443;
        else
            port = 80;
    }
    return (true);
}

char *api_request_string_url(const char *url, const char *method,
    json_group *payload, const char *headers, int *status, int timeout)
{
    if (ft_log_get_api_logging())
    {
        const char *log_url = "(null)";
        const char *log_method = "(null)";
        if (url)
            log_url = url;
        if (method)
            log_method = method;
        ft_log_debug("api_request_string_url %s %s",
            log_url, log_method);
    }
    bool tls;
    ft_string host;
    ft_string path;
    uint16_t port;
    if (!parse_url(url, tls, host, port, path))
        return (ft_nullptr);
    if (tls)
        return (api_request_string_tls(host.c_str(), port, method,
                                       path.c_str(), payload, headers,
                                       status, timeout));
    return (api_request_string_host(host.c_str(), port, method,
                                   path.c_str(), payload, headers,
                                   status, timeout));
}

json_group *api_request_json_url(const char *url, const char *method,
    json_group *payload, const char *headers, int *status, int timeout)
{
    char *body = api_request_string_url(url, method, payload,
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
