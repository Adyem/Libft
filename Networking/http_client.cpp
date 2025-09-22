#include "http_client.hpp"
#include "socket_class.hpp"
#include "ssl_wrapper.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include "../Errno/errno.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;

    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_send(socket_fd, buffer + total_sent, length - total_sent, 0);
        if (send_result <= 0)
        {
            if (send_result < 0)
                ft_errno = errno + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    return (0);
}

int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;

    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_ssl_write(ssl_connection, buffer + total_sent, length - total_sent);
        if (send_result <= 0)
        {
            ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    return (0);
}

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl, const char *custom_port)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
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
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
        return (-1);
    socket_fd = -1;
    result = -1;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
            FT_CLOSE_SOCKET(socket_fd);
            socket_fd = -1;
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
        return (-1);
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
        if (http_client_send_ssl_request(ssl_connection, request.c_str(), request.size()) != 0)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
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
        if (http_client_send_plain_request(socket_fd, request.c_str(), request.size()) != 0)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
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

int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl, const char *custom_port)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
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
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
        return (-1);
    socket_fd = -1;
    result = -1;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
            FT_CLOSE_SOCKET(socket_fd);
            socket_fd = -1;
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
        return (-1);
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
        if (http_client_send_ssl_request(ssl_connection, request.c_str(), request.size()) != 0)
        {
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
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
        if (http_client_send_plain_request(socket_fd, request.c_str(), request.size()) != 0)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
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

