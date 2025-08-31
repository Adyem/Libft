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
: _ctx(ft_nullptr), _ssl(ft_nullptr), _sock(-1), _host(host_c ? host_c : ""), _timeout(timeout_ms)
{
    if (!host_c)
        return;
    if (!OPENSSL_init_ssl(0, ft_nullptr))
        return;

    _ctx = SSL_CTX_new(TLS_client_method());
    if (!_ctx)
        return;

    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_string[6];
    std::snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host_c, port_string, &hints, &address_results) != 0)
        return;

    address_info = address_results;
    while (address_info != ft_nullptr)
    {
        _sock = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (_sock >= 0)
        {
            if (timeout_ms > 0)
            {
                struct timeval time_value;
                time_value.tv_sec = timeout_ms / 1000;
                time_value.tv_usec = (timeout_ms % 1000) * 1000;
                setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &time_value, sizeof(time_value));
                setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &time_value, sizeof(time_value));
            }
            if (nw_connect(_sock, address_info->ai_addr, static_cast<socklen_t>(address_info->ai_addrlen)) == 0)
                break;
            FT_CLOSE_SOCKET(_sock);
            _sock = -1;
        }
        address_info = address_info->ai_next;
    }
    if (address_results)
        freeaddrinfo(address_results);
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
        _ssl = ft_nullptr;
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
    return (_ssl != ft_nullptr);
}

char *api_tls_client::request(const char *method, const char *path, json_group *payload,
                              const char *headers, int *status)
{
    if (!_ssl || !method || !path)
        return (ft_nullptr);

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
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(_ssl, request.c_str(), request.size()) < 0)
        return (ft_nullptr);

    ft_string response;
    char buffer[1024];
    ssize_t bytes_received;
    const char *header_end_ptr = ft_nullptr;

    while (!header_end_ptr)
    {
        bytes_received = nw_ssl_read(_ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0)
            return (ft_nullptr);
        buffer[bytes_received] = '\0';
        response += buffer;
        header_end_ptr = strstr(response.c_str(), "\r\n\r\n");
    }

    if (status)
    {
        *status = -1;
        const char *space = strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }

    size_t header_len = static_cast<size_t>(header_end_ptr - response.c_str()) + 4;
    size_t content_length = 0;
    const char *content_length_ptr = strstr(response.c_str(), "Content-Length:");
    if (content_length_ptr)
    {
        content_length_ptr += strlen("Content-Length:");
        while (*content_length_ptr == ' ') content_length_ptr++;
        content_length = static_cast<size_t>(ft_atoi(content_length_ptr));
    }

    ft_string body(response.c_str() + header_len);
    while (body.size() < content_length)
    {
        bytes_received = nw_ssl_read(_ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0)
            return (ft_nullptr);
        buffer[bytes_received] = '\0';
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
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

