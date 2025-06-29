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

int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
#ifdef _WIN32
    if (bind((SOCKET)sockfd, addr, addrlen) == SOCKET_ERROR)
        return (-1);
#else
    if (bind(sockfd, addr, addrlen) == -1)
        return (-1);
#endif
    return (0);
}

int nw_listen(int sockfd, int backlog)
{
#ifdef _WIN32
    if (listen((SOCKET)sockfd, backlog) == SOCKET_ERROR)
        return (-1);
#else
    if (listen(sockfd, backlog) == -1)
        return (-1);
#endif
    return (0);
}

int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#ifdef _WIN32
    SOCKET new_fd = accept((SOCKET)sockfd, addr, addrlen);
    if (new_fd == INVALID_SOCKET)
        return (-1);
    return (int)new_fd;
#else
    int new_fd = accept(sockfd, addr, addrlen);
    if (new_fd == -1)
        return (-1);
    return (new_fd);
#endif
}

int nw_socket(int domain, int type, int protocol)
{
#ifdef _WIN32
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
    return (int)sockfd;
#else
    int sockfd = socket(domain, type, protocol);
    if (sockfd == -1)
        return (-1);
    return (sockfd);
#endif
}

int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
#ifdef _WIN32
    if (connect((SOCKET)sockfd, addr, addrlen) == SOCKET_ERROR)
        return (-1);
#else
    if (connect(sockfd, addr, addrlen) == -1)
        return (-1);
#endif
    return (0);
}

ssize_t nw_send(int sockfd, const void *buf, size_t len, int flags)
{
#ifdef _WIN32
    int ret = ::send((SOCKET)sockfd,
                     static_cast<const char*>(buf),
                     static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
        return (-1);
    return (ret);
#else
    ssize_t ret = ::send(sockfd, buf, len, flags);
    return (ret);
#endif
}

ssize_t nw_recv(int sockfd, void *buf, size_t len, int flags)
{
#ifdef _WIN32
    int ret = ::recv((SOCKET)sockfd,
                     static_cast<char*>(buf),
                     static_cast<int>(len), flags);
    if (ret == SOCKET_ERROR)
        return (-1);
    return (ret);
#else
    ssize_t ret = ::recv(sockfd, buf, len, flags);
    return (ret);
#endif
}
