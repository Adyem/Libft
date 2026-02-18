#include <stdlib.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
#include "socket_class.hpp"
#include "socket_handle.hpp"
#include "../CPP_class/class_nullptr.hpp"

#ifdef _WIN32
static inline int bind_platform(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    if (bind(static_cast<SOCKET>(socket_file_descriptor), address_value, address_length) == SOCKET_ERROR)
        return (-1);
    return (0);
}

static inline int listen_platform(int socket_file_descriptor, int backlog_value)
{
    if (listen(static_cast<SOCKET>(socket_file_descriptor), backlog_value) == SOCKET_ERROR)
        return (-1);
    return (0);
}

static inline int accept_platform(int socket_file_descriptor, struct sockaddr *address_value, socklen_t *address_length)
{
    SOCKET new_file_descriptor;

    new_file_descriptor = accept(static_cast<SOCKET>(socket_file_descriptor), address_value, address_length);
    if (new_file_descriptor == INVALID_SOCKET)
        return (-1);
    return (static_cast<int>(new_file_descriptor));
}

static inline int socket_platform(int domain_value, int type_value, int protocol_value)
{
    SOCKET socket_file_descriptor;

    if (ft_socket_runtime_acquire() != FT_ERR_SUCCESS)
        return (-1);
    socket_file_descriptor = socket(domain_value, type_value, protocol_value);
    if (socket_file_descriptor == INVALID_SOCKET)
    {
        ft_socket_runtime_release();
        return (-1);
    }
    return (static_cast<int>(socket_file_descriptor));
}

static inline int connect_platform(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    if (connect(static_cast<SOCKET>(socket_file_descriptor), address_value, address_length) == SOCKET_ERROR)
        return (-1);
    return (0);
}

static inline ssize_t send_platform(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags)
{
    int send_result;

    send_result = ::send(static_cast<SOCKET>(socket_file_descriptor), static_cast<const char *>(buffer_value), static_cast<int>(buffer_length), send_flags);
    if (send_result == SOCKET_ERROR)
        return (-1);
    return (send_result);
}

static inline ssize_t recv_platform(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags)
{
    int receive_result;

    receive_result = ::recv(static_cast<SOCKET>(socket_file_descriptor), static_cast<char *>(buffer_value), static_cast<int>(buffer_length), receive_flags);
    if (receive_result == SOCKET_ERROR)
        return (-1);
    return (receive_result);
}

static inline int shutdown_platform(int socket_file_descriptor, int shutdown_mode)
{
    if (::shutdown(static_cast<SOCKET>(socket_file_descriptor), shutdown_mode) == SOCKET_ERROR)
        return (-1);
    return (0);
}

static inline ssize_t sendto_platform(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags,
    const struct sockaddr *destination_address, socklen_t address_length)
{
    int send_result;

    send_result = ::sendto(static_cast<SOCKET>(socket_file_descriptor), static_cast<const char *>(buffer_value),
            static_cast<int>(buffer_length), send_flags, destination_address, address_length);
    if (send_result == SOCKET_ERROR)
        return (-1);
    return (send_result);
}

static inline ssize_t recvfrom_platform(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags,
    struct sockaddr *source_address, socklen_t *address_length)
{
    int receive_result;

    receive_result = ::recvfrom(static_cast<SOCKET>(socket_file_descriptor), static_cast<char *>(buffer_value),
            static_cast<int>(buffer_length), receive_flags, source_address, address_length);
    if (receive_result == SOCKET_ERROR)
        return (-1);
    return (receive_result);
}
#else
static inline int bind_platform(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    if (bind(socket_file_descriptor, address_value, address_length) == -1)
        return (-1);
    return (0);
}

static inline int listen_platform(int socket_file_descriptor, int backlog_value)
{
    if (listen(socket_file_descriptor, backlog_value) == -1)
        return (-1);
    return (0);
}

static inline int accept_platform(int socket_file_descriptor, struct sockaddr *address_value, socklen_t *address_length)
{
    int new_file_descriptor;

    new_file_descriptor = accept(socket_file_descriptor, address_value, address_length);
    if (new_file_descriptor == -1)
        return (-1);
    return (new_file_descriptor);
}

static inline int socket_platform(int domain_value, int type_value, int protocol_value)
{
    int socket_file_descriptor;

    socket_file_descriptor = socket(domain_value, type_value, protocol_value);
    if (socket_file_descriptor == -1)
        return (-1);
    return (socket_file_descriptor);
}

static inline int connect_platform(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    if (connect(socket_file_descriptor, address_value, address_length) == -1)
        return (-1);
    return (0);
}

static inline ssize_t send_platform(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags)
{
    ssize_t send_result;

    send_result = ::send(socket_file_descriptor, buffer_value, buffer_length, send_flags);
    if (send_result == -1)
        return (-1);
    return (send_result);
}

static inline ssize_t recv_platform(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags)
{
    ssize_t receive_result;

    receive_result = ::recv(socket_file_descriptor, buffer_value, buffer_length, receive_flags);
    if (receive_result == -1)
        return (-1);
    return (receive_result);
}

static inline int shutdown_platform(int socket_file_descriptor, int shutdown_mode)
{
    if (::shutdown(socket_file_descriptor, shutdown_mode) == -1)
        return (-1);
    return (0);
}

static inline ssize_t sendto_platform(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags,
    const struct sockaddr *destination_address, socklen_t address_length)
{
    ssize_t send_result;

    send_result = ::sendto(socket_file_descriptor, buffer_value, buffer_length, send_flags, destination_address, address_length);
    if (send_result == -1)
        return (-1);
    return (send_result);
}

static inline ssize_t recvfrom_platform(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags,
    struct sockaddr *source_address, socklen_t *address_length)
{
    ssize_t receive_result;

    receive_result = ::recvfrom(socket_file_descriptor, buffer_value, buffer_length, receive_flags, source_address, address_length);
    if (receive_result == -1)
        return (-1);
    return (receive_result);
}
#endif

int nw_bind(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    return (bind_platform(socket_file_descriptor, address_value, address_length));
}

int nw_listen(int socket_file_descriptor, int backlog_value)
{
    return (listen_platform(socket_file_descriptor, backlog_value));
}

int nw_accept(int socket_file_descriptor, struct sockaddr *address_value, socklen_t *address_length)
{
    int accepted_file_descriptor;

    accepted_file_descriptor = accept_platform(socket_file_descriptor, address_value, address_length);
    if (accepted_file_descriptor < 0)
        return (-1);
#ifdef _WIN32
    if (ft_socket_runtime_acquire() != FT_ERR_SUCCESS)
    {
        nw_close(accepted_file_descriptor);
        return (-1);
    }
#endif
    return (accepted_file_descriptor);
}

static t_nw_socket_hook g_nw_socket_hook = socket_platform;

void nw_set_socket_hook(t_nw_socket_hook hook)
{
    if (hook != ft_nullptr)
    {
        g_nw_socket_hook = hook;
        return ;
    }
    g_nw_socket_hook = socket_platform;
    return ;
}

int nw_socket(int domain_value, int type_value, int protocol_value)
{
    return (g_nw_socket_hook(domain_value, type_value, protocol_value));
}

int nw_connect(int socket_file_descriptor, const struct sockaddr *address_value, socklen_t address_length)
{
    return (connect_platform(socket_file_descriptor, address_value, address_length));
}

ssize_t nw_send(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags)
{
    return (send_platform(socket_file_descriptor, buffer_value, buffer_length, send_flags));
}

ssize_t nw_recv(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags)
{
    return (recv_platform(socket_file_descriptor, buffer_value, buffer_length, receive_flags));
}

ssize_t nw_sendto(int socket_file_descriptor, const void *buffer_value, size_t buffer_length, int send_flags,
    const struct sockaddr *destination_address, socklen_t address_length)
{
    return (sendto_platform(socket_file_descriptor, buffer_value, buffer_length, send_flags, destination_address, address_length));
}

ssize_t nw_recvfrom(int socket_file_descriptor, void *buffer_value, size_t buffer_length, int receive_flags,
    struct sockaddr *source_address, socklen_t *address_length)
{
    return (recvfrom_platform(socket_file_descriptor, buffer_value, buffer_length, receive_flags, source_address, address_length));
}

int nw_close(int socket_file_descriptor)
{
#ifdef _WIN32
    if (closesocket(static_cast<SOCKET>(socket_file_descriptor)) == SOCKET_ERROR)
        return (-1);
    ft_socket_runtime_release();
    return (0);
#else
    if (close(socket_file_descriptor) == -1)
        return (-1);
    return (0);
#endif
}

int nw_shutdown(int socket_file_descriptor, int shutdown_mode)
{
    return (shutdown_platform(socket_file_descriptor, shutdown_mode));
}

int nw_inet_pton(int family_value, const char *ip_address, void *destination)
{
    int conversion_result;

    if (ip_address == ft_nullptr || destination == ft_nullptr)
        return (-1);
    conversion_result = inet_pton(family_value, ip_address, destination);
    if (conversion_result == 1)
        return (1);
    if (conversion_result == 0)
        return (0);
    return (-1);
}
