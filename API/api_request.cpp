#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstring>

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
