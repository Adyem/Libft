#include "http_client.hpp"
#include "socket_class.hpp"
#include "ssl_wrapper.hpp"
#include <cstring>
#include <cstdio>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    const char *port_string;
    int socket_fd;
    ft_string request;
    char buffer[1024];
    ssize_t bytes_received;
    SSL_CTX *ssl_context;
    SSL *ssl_connection;
    int result;

    response.clear();
    std::memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
        return (-1);
    socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (socket_fd < 0)
    {
        freeaddrinfo(address_info);
        return (-1);
    }
    result = nw_connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
    freeaddrinfo(address_info);
    if (result < 0)
    {
        FT_CLOSE_SOCKET(socket_fd);
        return (-1);
    }
    request.append("GET ");
    request.append(path);
    request.append(" HTTP/1.1\r\nHost: ");
    request.append(host);
    request.append("\r\nConnection: close\r\n\r\n");
    if (use_ssl)
    {
        SSL_library_init();
        ssl_context = SSL_CTX_new(SSLv23_client_method());
        if (ssl_context == NULL)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        ssl_connection = SSL_new(ssl_context);
        if (ssl_connection == NULL)
        {
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        SSL_set_fd(ssl_connection, socket_fd);
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        nw_ssl_write(ssl_connection, request.c_str(), request.size());
        bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        while (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            response.append(buffer);
            bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        }
        SSL_shutdown(ssl_connection);
        SSL_free(ssl_connection);
        SSL_CTX_free(ssl_context);
    }
    else
    {
        nw_send(socket_fd, request.c_str(), request.size(), 0);
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        while (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            response.append(buffer);
            bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        }
    }
    FT_CLOSE_SOCKET(socket_fd);
    return (0);
}

int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    const char *port_string;
    int socket_fd;
    ft_string request;
    char buffer[1024];
    char length_string[32];
    ssize_t bytes_received;
    SSL_CTX *ssl_context;
    SSL *ssl_connection;
    int result;

    response.clear();
    std::memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
        return (-1);
    socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (socket_fd < 0)
    {
        freeaddrinfo(address_info);
        return (-1);
    }
    result = nw_connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
    freeaddrinfo(address_info);
    if (result < 0)
    {
        FT_CLOSE_SOCKET(socket_fd);
        return (-1);
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
        SSL_library_init();
        ssl_context = SSL_CTX_new(SSLv23_client_method());
        if (ssl_context == NULL)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        ssl_connection = SSL_new(ssl_context);
        if (ssl_connection == NULL)
        {
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        SSL_set_fd(ssl_connection, socket_fd);
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        nw_ssl_write(ssl_connection, request.c_str(), request.size());
        bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        while (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            response.append(buffer);
            bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        }
        SSL_shutdown(ssl_connection);
        SSL_free(ssl_connection);
        SSL_CTX_free(ssl_context);
    }
    else
    {
        nw_send(socket_fd, request.c_str(), request.size(), 0);
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        while (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            response.append(buffer);
            bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        }
    }
    FT_CLOSE_SOCKET(socket_fd);
    return (0);
}

