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

#ifdef _WIN32
static inline int bind_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return (bind(static_cast<SOCKET>(sockfd), addr, len) == SOCKET_ERROR) ? -1 : 0;
}

static inline int listen_platform(int sockfd, int backlog)
{
    return (listen(static_cast<SOCKET>(sockfd), backlog) == SOCKET_ERROR) ? -1 : 0;
}

static inline int accept_platform(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    SOCKET new_fd = accept(static_cast<SOCKET>(sockfd), addr, addrlen);
    if (new_fd == INVALID_SOCKET)
        return (-1);
    return (static_cast<int>(new_fd));
}

static inline int socket_platform(int domain, int type, int protocol)
{
    static int initialized = 0;
    if (!initialized)
    {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2,2), &data) != 0)
            return (-1);
        initialized = 1;
    }
    SOCKET sockfd = socket(domain, type, protocol);
    if (sockfd == INVALID_SOCKET)
        return (-1);
    return (static_cast<int>(sockfd));
}

static inline int connect_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return (connect(static_cast<SOCKET>(sockfd), addr, len) == SOCKET_ERROR) ? -1 : 0;
}

static inline ssize_t send_platform(int sockfd, const void *buf, size_t len, int flags)
{
    int ret = ::send(static_cast<SOCKET>(sockfd), static_cast<const char*>(buf), static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
        return (-1);
    return (ret);
}

static inline ssize_t recv_platform(int sockfd, void *buf, size_t len, int flags)
{
    int ret = ::recv(static_cast<SOCKET>(sockfd), static_cast<char*>(buf), static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
        return (-1);
    return (ret);
}
#else
static inline int bind_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return (bind(sockfd, addr, len) == -1) ? -1 : 0;
}

static inline int listen_platform(int sockfd, int backlog)
{
    return (listen(sockfd, backlog) == -1) ? -1 : 0;
}

static inline int accept_platform(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int new_fd = accept(sockfd, addr, addrlen);
    if (new_fd == -1)
        return (-1);
    return (new_fd);
}

static inline int socket_platform(int domain, int type, int protocol)
{
    int sockfd = socket(domain, type, protocol);
    if (sockfd == -1)
        return (-1);
    return (sockfd);
}

static inline int connect_platform(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return (connect(sockfd, addr, len) == -1) ? -1 : 0;
}

static inline ssize_t send_platform(int sockfd, const void *buf, size_t len, int flags)
{
    return (::send(sockfd, buf, len, flags));
}

static inline ssize_t recv_platform(int sockfd, void *buf, size_t len, int flags)
{
    return (::recv(sockfd, buf, len, flags));
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
    return (accept_platform(sockfd, addr, addrlen));
}

int nw_socket(int domain, int type, int protocol)
{
    return (socket_platform(domain, type, protocol));
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
