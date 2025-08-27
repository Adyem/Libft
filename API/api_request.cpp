#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstring>
#include <cstdio>
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
    SocketConfig config;
    config.type = SocketType::CLIENT;
    config.ip = ip;
    config.port = port;
    config.recv_timeout = timeout;
    config.send_timeout = timeout;

    ft_socket sock(config);
    if (sock.get_error())
        return (NULL);

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
        char *tmp = json_write_to_string(payload);
        if (!tmp)
            return (NULL);
        body_string = tmp;
        cma_free(tmp);
        request += "\r\nContent-Type: application/json";
        char *len = cma_itoa(static_cast<int>(body_string.size()));
        if (!len)
            return (NULL);
        request += "\r\nContent-Length: ";
        request += len;
        cma_free(len);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (sock.send_all(request.c_str(), request.size(), 0) < 0)
        return (NULL);

    char buffer[1024];
    ft_string response;
    ssize_t bytes;

    while ((bytes = sock.receive_data(buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes] = '\0';
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
        return (NULL);
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
        return (NULL);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

char *api_request_string_host(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *p;
    int sock = -1;
    ft_string request;
    ft_string body_string;
    ft_string response;
    char *ret = NULL;
    char buffer[1024];
    ssize_t bytes;
    const char *body = NULL;
    char port_str[6];

    if (!host || !method || !path)
        return (NULL);
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    std::snprintf(port_str, sizeof(port_str), "%u", port);
    if (getaddrinfo(host, port_str, &hints, &res) != 0)
        goto cleanup;

    for (p = res; p != NULL; p = p->ai_next)
    {
        sock = nw_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0)
            continue;
        if (timeout > 0)
        {
            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        }
        if (nw_connect(sock, p->ai_addr, static_cast<socklen_t>(p->ai_addrlen)) == 0)
            break;
        FT_CLOSE_SOCKET(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    res = NULL;
    if (sock < 0)
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
        char *tmp = json_write_to_string(payload);
        if (!tmp)
            goto cleanup;
        body_string = tmp;
        cma_free(tmp);
        request += "\r\nContent-Type: application/json";
        char *len = cma_itoa(static_cast<int>(body_string.size()));
        if (!len)
            goto cleanup;
        request += "\r\nContent-Length: ";
        request += len;
        cma_free(len);
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (nw_send(sock, request.c_str(), request.size(), 0) < 0)
        goto cleanup;

    while ((bytes = nw_recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes] = '\0';
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
    ret = cma_strdup(body);

cleanup:
    if (sock >= 0)
        FT_CLOSE_SOCKET(sock);
    if (res)
        freeaddrinfo(res);
    return (ret);
}

json_group *api_request_json_host(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string_host(host, port, method, path, payload,
                                        headers, status, timeout);
    if (!body)
        return (NULL);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}
