#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include <cstring>
#include <cstdio>
#include <string>
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
    SocketConfig config;
    config._type = SocketType::CLIENT;
    config._ip = ip;
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_socket socket_wrapper(config);
    if (socket_wrapper.get_error())
        return (ft_nullptr);

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
            return (ft_nullptr);
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        char *length_string = cma_itoa(static_cast<int>(body_string.size()));
        if (!length_string)
            return (ft_nullptr);
        request += "\r\nContent-Length: ";
        request += length_string;
        cma_free(length_string);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (socket_wrapper.send_all(request.c_str(), request.size(), 0) < 0)
        return (ft_nullptr);

    char buffer[1024];
    ft_string response;
    ssize_t bytes_received;

    while ((bytes_received = socket_wrapper.receive_data(buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        response += buffer;
    }
    if (status)
    {
        *status = -1;
        const char *space = strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    const char *body = strstr(response.c_str(), "\r\n\r\n");
    if (!body)
        return (ft_nullptr);
    body += 4;
    return (cma_strdup(body));
}

json_group *api_request_json(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string(ip, port, method, path, payload,
                                   headers, status, timeout);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
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
        return (ft_nullptr);
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    std::snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host, port_string, &hints, &address_results) != 0)
        goto cleanup;

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

    if (nw_send(socket_fd, request.c_str(), request.size(), 0) < 0)
        goto cleanup;

    while ((bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        response += buffer;
    }
    if (status)
    {
        *status = -1;
        const char *space = strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    body = strstr(response.c_str(), "\r\n\r\n");
    if (!body)
        goto cleanup;
    body += 4;
    result = cma_strdup(body);

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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
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
    return (result);
}

static bool parse_url(const char *url, bool &tls, std::string &host,
                      uint16_t &port, std::string &path)
{
    if (!url)
        return (false);
    const char *scheme_end = std::strstr(url, "://");
    if (!scheme_end)
        return (false);
    std::string scheme(url, scheme_end - url);
    tls = (scheme == "https");
    const char *host_start = scheme_end + 3;
    const char *path_start = std::strchr(host_start, '/');
    if (path_start)
        path.assign(path_start);
    else
        path = "/";
    std::string hostport;
    if (path_start)
        hostport.assign(host_start, path_start - host_start);
    else
        hostport = host_start;
    const char *colon = std::strchr(hostport.c_str(), ':');
    if (colon)
    {
        host.assign(hostport.c_str(), colon - hostport.c_str());
        port = static_cast<uint16_t>(ft_atoi(colon + 1));
    }
    else
    {
        host = hostport;
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
    std::string host;
    std::string path;
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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}
