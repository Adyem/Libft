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
# include <errno.h>
#endif
#include "socket_class.hpp"
#include "socket_handle.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

#ifdef _WIN32
static inline int bind_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (bind(static_cast<SOCKET>(sockfd), addr, len) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int listen_platform(int sockfd, int backlog)
{
    if (listen(static_cast<SOCKET>(sockfd), backlog) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int accept_platform(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    SOCKET new_fd = accept(static_cast<SOCKET>(sockfd), addr, addrlen);
    if (new_fd == INVALID_SOCKET)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<int>(new_fd));
}

static inline int socket_platform(int domain, int type, int protocol)
{
    if (ft_socket_runtime_acquire() != ER_SUCCESS)
    {
        return (-1);
    }
    SOCKET sockfd = socket(domain, type, protocol);
    if (sockfd == INVALID_SOCKET)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        ft_socket_runtime_release();
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<int>(sockfd));
}

static inline int connect_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (connect(static_cast<SOCKET>(sockfd), addr, len) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline ssize_t send_platform(int sockfd, const void *buf, size_t len, int flags)
{
    int ret = ::send(static_cast<SOCKET>(sockfd), static_cast<const char*>(buf), static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (ret);
}

static inline ssize_t recv_platform(int sockfd, void *buf, size_t len, int flags)
{
    int ret = ::recv(static_cast<SOCKET>(sockfd), static_cast<char*>(buf), static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (ret);
}

static inline ssize_t sendto_platform(int sockfd, const void *buf, size_t len, int flags,
                                      const struct sockaddr *dest_addr, socklen_t addrlen)
{
    int ret = ::sendto(static_cast<SOCKET>(sockfd), static_cast<const char*>(buf),
                       static_cast<int>(len), flags, dest_addr, addrlen);
    if (ret == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (ret);
}

static inline ssize_t recvfrom_platform(int sockfd, void *buf, size_t len, int flags,
                                        struct sockaddr *src_addr, socklen_t *addrlen)
{
    int ret = ::recvfrom(static_cast<SOCKET>(sockfd), static_cast<char*>(buf),
                         static_cast<int>(len), flags, src_addr, addrlen);
    if (ret == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (ret);
}
#else
static inline int bind_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (bind(sockfd, addr, len) == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int listen_platform(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int accept_platform(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int new_fd = accept(sockfd, addr, addrlen);
    if (new_fd == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (new_fd);
}

static inline int socket_platform(int domain, int type, int protocol)
{
    int sockfd = socket(domain, type, protocol);
    if (sockfd == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (sockfd);
}

static inline int connect_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (connect(sockfd, addr, len) == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline ssize_t send_platform(int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t result;

    result = ::send(sockfd, buf, len, flags);
    if (result == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

static inline ssize_t recv_platform(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t result;

    result = ::recv(sockfd, buf, len, flags);
    if (result == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

static inline ssize_t sendto_platform(int sockfd, const void *buf, size_t len, int flags,
                                      const struct sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t result;

    result = ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (result == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

static inline ssize_t recvfrom_platform(int sockfd, void *buf, size_t len, int flags,
                                        struct sockaddr *src_addr, socklen_t *addrlen)
{
    ssize_t result;

    result = ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    if (result == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (result);
}
#endif

int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return (bind_platform(sockfd, addr, addrlen));
}

int nw_listen(int sockfd, int backlog)
{
    return (listen_platform(sockfd, backlog));
}

int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int accepted_fd;
    int acquire_error;

    accepted_fd = accept_platform(sockfd, addr, addrlen);
    if (accepted_fd < 0)
    {
        return (-1);
    }
    if (ft_socket_runtime_acquire() != ER_SUCCESS)
    {
        acquire_error = ft_errno;
        nw_close(accepted_fd);
        ft_errno = acquire_error;
        return (-1);
    }
    return (accepted_fd);
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

int nw_socket(int domain, int type, int protocol)
{
    return (g_nw_socket_hook(domain, type, protocol));
}

int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return (connect_platform(sockfd, addr, addrlen));
}

ssize_t nw_send(int sockfd, const void *buf, size_t len, int flags)
{
    return (send_platform(sockfd, buf, len, flags));
}

ssize_t nw_recv(int sockfd, void *buf, size_t len, int flags)
{
    return (recv_platform(sockfd, buf, len, flags));
}

ssize_t nw_sendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen)
{
    return (sendto_platform(sockfd, buf, len, flags, dest_addr, addrlen));
}

ssize_t nw_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen)
{
    return (recvfrom_platform(sockfd, buf, len, flags, src_addr, addrlen));
}

int nw_close(int sockfd)
{
#ifdef _WIN32
    int previous_error;

    previous_error = ft_errno;
    if (closesocket(static_cast<SOCKET>(sockfd)) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        if (ft_errno == ER_SUCCESS)
        {
            ft_errno = FT_ERR_SOCKET_CLOSE_FAILED;
        }
        return (-1);
    }
    ft_socket_runtime_release();
    if (ft_errno != ER_SUCCESS)
    {
        return (-1);
    }
    ft_errno = previous_error;
    return (0);
#else
    int previous_error;

    previous_error = ft_errno;
    if (close(sockfd) == -1)
    {
        ft_errno = ft_map_system_error(errno);
        if (ft_errno == ER_SUCCESS)
        {
            ft_errno = FT_ERR_SOCKET_CLOSE_FAILED;
        }
        return (-1);
    }
    ft_errno = previous_error;
    return (0);
#endif
}

int nw_inet_pton(int family, const char *ip_address, void *destination)
{
    int result;

    if (ip_address == ft_nullptr || destination == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    result = inet_pton(family, ip_address, destination);
    if (result == 1)
    {
        ft_errno = ER_SUCCESS;
        return (1);
    }
    if (result == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
#ifdef _WIN32
    {
        int error_code;

        error_code = WSAGetLastError();
        if (error_code != 0)
            ft_errno = ft_map_system_error(error_code);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
    }
#else
    if (errno != 0)
        ft_errno = ft_map_system_error(errno);
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
#endif
    return (-1);
}
