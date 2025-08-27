#include "api.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
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
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int sock = -1;
    char *ret = NULL;
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *p;
    ft_string request;
    ft_string body_string;
    ft_string response;
    const char *body = NULL;

    if (!host || !method || !path)
        return (NULL);
    if (!OPENSSL_init_ssl(0, NULL))
        return (NULL);

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
        goto cleanup;

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_str[6];
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

    ssl = SSL_new(ctx);
    if (!ssl)
        goto cleanup;
    if (SSL_set_fd(ssl, sock) != 1)
        goto cleanup;
    if (SSL_connect(ssl) <= 0)
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

    if (ssl_send_all(ssl, request.c_str(), request.size()) < 0)
        goto cleanup;

    char buffer[1024];
    ssize_t bytes;
    while ((bytes = nw_ssl_read(ssl, buffer, sizeof(buffer) - 1)) > 0)
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
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock >= 0)
        FT_CLOSE_SOCKET(sock);
    if (ctx)
        SSL_CTX_free(ctx);
    if (res)
        freeaddrinfo(res);
    return (ret);
}

json_group *api_request_json_tls(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout)
{
    char *body = api_request_string_tls(host, port, method, path, payload,
                                        headers, status, timeout);
    if (!body)
        return (NULL);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}
