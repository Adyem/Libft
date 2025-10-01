#include "http_client.hpp"
#include "socket_class.hpp"
#include "networking.hpp"
#include "ssl_wrapper.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <openssl/x509v3.h>
#include "../Errno/errno.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

static int http_client_wait_for_socket_ready(int socket_fd, bool wait_for_write)
{
    int poll_descriptor;
    int poll_result;

    if (socket_fd < 0)
    {
        ft_errno = SOCKET_SEND_FAILED;
        return (-1);
    }
    poll_descriptor = socket_fd;
    if (wait_for_write != false)
        poll_result = nw_poll(NULL, 0, &poll_descriptor, 1, 1000);
    else
        poll_result = nw_poll(&poll_descriptor, 1, NULL, 0, 1000);
    if (poll_result < 0)
    {
#ifdef _WIN32
        ft_errno = WSAGetLastError() + ERRNO_OFFSET;
#else
        ft_errno = errno + ERRNO_OFFSET;
#endif
        return (-1);
    }
    if (poll_result == 0)
        return (1);
    return (0);
}

static void http_client_set_resolve_error(int resolver_status)
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

static int http_client_initialize_ssl(int socket_fd, const char *host, SSL_CTX **ssl_context, SSL **ssl_connection)
{
    SSL_CTX *local_context;
    SSL *local_connection;

    if (ssl_context == NULL || ssl_connection == NULL)
    {
        ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    *ssl_context = NULL;
    *ssl_connection = NULL;
    SSL_library_init();
    local_context = SSL_CTX_new(TLS_client_method());
    if (local_context == NULL)
    {
        ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    SSL_CTX_set_verify(local_context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(local_context) != 1)
    {
        SSL_CTX_free(local_context);
        ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    local_connection = SSL_new(local_context);
    if (local_connection == NULL)
    {
        SSL_CTX_free(local_context);
        ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    if (host != NULL && host[0] != '\0')
    {
        long control_result;

        control_result = SSL_ctrl(local_connection, SSL_CTRL_SET_TLSEXT_HOSTNAME,
            TLSEXT_NAMETYPE_host_name, const_cast<char *>(host));
        if (control_result != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = SOCKET_CONNECT_FAILED;
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        X509_VERIFY_PARAM *verify_params;

        verify_params = SSL_get0_param(local_connection);
        if (verify_params == NULL)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = SOCKET_CONNECT_FAILED;
            return (-1);
        }
        X509_VERIFY_PARAM_set_hostflags(verify_params, 0);
        if (X509_VERIFY_PARAM_set1_host(verify_params, host, 0) != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            ft_errno = SOCKET_CONNECT_FAILED;
            return (-1);
        }
#endif
    }
    if (SSL_set_fd(local_connection, socket_fd) != 1)
    {
        SSL_free(local_connection);
        SSL_CTX_free(local_context);
        ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    *ssl_context = local_context;
    *ssl_connection = local_connection;
    return (0);
}

int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;

    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_send(socket_fd, buffer + total_sent, length - total_sent, 0);
        if (send_result < 0)
        {
            int wait_result;

#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEINTR)
                continue ;
            if (last_error == WSAEWOULDBLOCK)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            ft_errno = last_error + ERRNO_OFFSET;
#else
            if (errno == EINTR)
                continue ;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            ft_errno = errno + ERRNO_OFFSET;
#endif
            return (-1);
        }
        if (send_result == 0)
        {
            ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_socket_after_send(socket_fd) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;
    int socket_fd;

    socket_fd = -1;
    if (ssl_connection != NULL)
        socket_fd = SSL_get_fd(ssl_connection);
    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_ssl_write(ssl_connection, buffer + total_sent, length - total_sent);
        if (send_result <= 0)
        {
            int ssl_error;
            int wait_result;

            ssl_error = SSL_get_error(ssl_connection, static_cast<int>(send_result));
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
            {
                bool wait_for_write;

                wait_for_write = (ssl_error == SSL_ERROR_WANT_WRITE);
                wait_result = http_client_wait_for_socket_ready(socket_fd, wait_for_write);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            if (ssl_error == SSL_ERROR_SYSCALL)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (last_error == WSAEINTR)
                    continue ;
                if (last_error == WSAEWOULDBLOCK)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error != 0)
                {
                    ft_errno = last_error + ERRNO_OFFSET;
                    return (-1);
                }
#else
                if (errno == EINTR)
                    continue ;
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (errno != 0)
                {
                    ft_errno = errno + ERRNO_OFFSET;
                    return (-1);
                }
#endif
            }
            ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_ssl_after_send(ssl_connection) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
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
    int resolver_status;
    int last_socket_error;

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
    resolver_status = getaddrinfo(host, port_string, &address_hints, &address_info);
    if (resolver_status != 0)
    {
        http_client_set_resolve_error(resolver_status);
        return (-1);
    }
    socket_fd = -1;
    result = -1;
    last_socket_error = 0;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
            FT_CLOSE_SOCKET(socket_fd);
            socket_fd = -1;
        }
        else
        {
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
    {
        if (last_socket_error != 0)
            ft_errno = last_socket_error + ERRNO_OFFSET;
        else
            ft_errno = SOCKET_CONNECT_FAILED;
        return (-1);
    }
    request.append("GET ");
    request.append(path);
    request.append(" HTTP/1.1\r\nHost: ");
    request.append(host);
    request.append("\r\nConnection: close\r\n\r\n");
    if (use_ssl)
    {
        if (http_client_initialize_ssl(socket_fd, host, &ssl_context, &ssl_connection) != 0)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            ft_errno = SOCKET_CONNECT_FAILED;
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (SSL_get_verify_result(ssl_connection) != X509_V_OK)
        {
            ft_errno = SOCKET_CONNECT_FAILED;
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
#endif
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
        if (bytes_received < 0)
        {
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
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
        if (bytes_received < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            FT_CLOSE_SOCKET(socket_fd);
            if (last_error != 0)
                ft_errno = last_error + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_RECEIVE_FAILED;
#else
            int last_error;

            last_error = errno;
            FT_CLOSE_SOCKET(socket_fd);
            if (last_error != 0)
                ft_errno = last_error + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_RECEIVE_FAILED;
#endif
            return (-1);
        }
    }
    FT_CLOSE_SOCKET(socket_fd);
    ft_errno = ER_SUCCESS;
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
    int resolver_status;
    int last_socket_error;

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
    resolver_status = getaddrinfo(host, port_string, &address_hints, &address_info);
    if (resolver_status != 0)
    {
        http_client_set_resolve_error(resolver_status);
        return (-1);
    }
    socket_fd = -1;
    result = -1;
    last_socket_error = 0;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
            FT_CLOSE_SOCKET(socket_fd);
            socket_fd = -1;
        }
        else
        {
#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            if (errno != 0)
                last_socket_error = errno;
#endif
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
    {
        if (last_socket_error != 0)
            ft_errno = last_socket_error + ERRNO_OFFSET;
        else
            ft_errno = SOCKET_CONNECT_FAILED;
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
        if (http_client_initialize_ssl(socket_fd, host, &ssl_context, &ssl_connection) != 0)
        {
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
        result = SSL_connect(ssl_connection);
        if (result != 1)
        {
            ft_errno = SOCKET_CONNECT_FAILED;
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (SSL_get_verify_result(ssl_connection) != X509_V_OK)
        {
            ft_errno = SOCKET_CONNECT_FAILED;
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
        }
#endif
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
        if (bytes_received < 0)
        {
            SSL_shutdown(ssl_connection);
            SSL_free(ssl_connection);
            SSL_CTX_free(ssl_context);
            FT_CLOSE_SOCKET(socket_fd);
            return (-1);
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
        if (bytes_received < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            FT_CLOSE_SOCKET(socket_fd);
            if (last_error != 0)
                ft_errno = last_error + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_RECEIVE_FAILED;
#else
            int last_error;

            last_error = errno;
            FT_CLOSE_SOCKET(socket_fd);
            if (last_error != 0)
                ft_errno = last_error + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_RECEIVE_FAILED;
#endif
            return (-1);
        }
    }
    FT_CLOSE_SOCKET(socket_fd);
    ft_errno = ER_SUCCESS;
    return (0);
}

