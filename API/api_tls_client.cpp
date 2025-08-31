#include "api_tls_client.hpp"
#include <cstring>
#include <cstdio>
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
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

api_tls_client::api_tls_client(const char *host_c, uint16_t port, int timeout_ms)
: _ctx(NULL), _ssl(NULL), _sock(-1), _host(host_c ? host_c : ""), _timeout(timeout_ms)
{
    if (!host_c)
        return;
    if (!OPENSSL_init_ssl(0, NULL))
        return;

    _ctx = SSL_CTX_new(TLS_client_method());
    if (!_ctx)
        return;

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *p;
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_str[6];
    std::snprintf(port_str, sizeof(port_str), "%u", port);
    if (getaddrinfo(host_c, port_str, &hints, &res) != 0)
        return;

    for (p = res; p != NULL; p = p->ai_next)
    {
        _sock = nw_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_sock < 0)
            continue;
        if (timeout_ms > 0)
        {
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        }
        if (nw_connect(_sock, p->ai_addr, static_cast<socklen_t>(p->ai_addrlen)) == 0)
            break;
        FT_CLOSE_SOCKET(_sock);
        _sock = -1;
    }
    if (res)
        freeaddrinfo(res);
    if (_sock < 0)
        return;

    _ssl = SSL_new(_ctx);
    if (!_ssl)
        return;
    if (SSL_set_fd(_ssl, _sock) != 1)
        return;
    if (SSL_connect(_ssl) <= 0)
    {
        SSL_free(_ssl);
        _ssl = NULL;
        return;
    }
}

api_tls_client::~api_tls_client()
{
    if (_ssl)
    {
        SSL_shutdown(_ssl);
        SSL_free(_ssl);
    }
    if (_sock >= 0)
        FT_CLOSE_SOCKET(_sock);
    if (_ctx)
        SSL_CTX_free(_ctx);
}

bool api_tls_client::is_valid() const
{
    return (_ssl != NULL);
}

char *api_tls_client::request(const char *method, const char *path, json_group *payload,
                              const char *headers, int *status)
{
    if (!_ssl || !method || !path)
        return (NULL);

    ft_string request(method);
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += _host.c_str();
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
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(_ssl, request.c_str(), request.size()) < 0)
        return (NULL);

    ft_string resp;
    char buffer[1024];
    ssize_t bytes;
    const char *header_end_ptr = NULL;

    while (!header_end_ptr)
    {
        bytes = nw_ssl_read(_ssl, buffer, sizeof(buffer) - 1);
        if (bytes <= 0)
            return (NULL);
        buffer[bytes] = '\0';
        resp += buffer;
        header_end_ptr = strstr(resp.c_str(), "\r\n\r\n");
    }

    if (status)
    {
        *status = -1;
        const char *space = strchr(resp.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }

    size_t header_len = static_cast<size_t>(header_end_ptr - resp.c_str()) + 4;
    size_t content_length = 0;
    const char *cl = strstr(resp.c_str(), "Content-Length:");
    if (cl)
    {
        cl += strlen("Content-Length:");
        while (*cl == ' ') cl++;
        content_length = static_cast<size_t>(ft_atoi(cl));
    }

    ft_string body(resp.c_str() + header_len);
    while (body.size() < content_length)
    {
        bytes = nw_ssl_read(_ssl, buffer, sizeof(buffer) - 1);
        if (bytes <= 0)
            return (NULL);
        buffer[bytes] = '\0';
        body += buffer;
    }

    return (cma_strdup(body.c_str()));
}

json_group *api_tls_client::request_json(const char *method, const char *path,
                                         json_group *payload,
                                         const char *headers, int *status)
{
    char *body = request(method, path, payload, headers, status);
    if (!body)
        return (NULL);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

