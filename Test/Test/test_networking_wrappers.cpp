#include "../../Networking/networking.hpp"
#include "../../Networking/ssl_wrapper.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#ifndef _WIN32
# include <dlfcn.h>
# include <errno.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <cstdarg>
# include <cstring>
#endif

extern int g_mock_ssl_write_should_fail;
extern int g_mock_ssl_read_should_fail;

#ifndef _WIN32
static int g_bind_should_fail = 0;
static int g_bind_fail_errno = EACCES;
static int g_listen_should_fail = 0;
static int g_listen_fail_errno = EINVAL;
static int g_accept_should_fail = 0;
static int g_accept_fail_errno = EMFILE;
static int g_socket_should_fail = 0;
static int g_socket_fail_errno = ENFILE;
static int g_connect_should_fail = 0;
static int g_connect_fail_errno = ECONNREFUSED;
static int g_send_should_fail = 0;
static int g_send_fail_errno = EPIPE;
static int g_recv_should_fail = 0;
static int g_recv_fail_errno = ECONNRESET;
static int g_sendto_should_fail = 0;
static int g_sendto_fail_errno = EDESTADDRREQ;
static int g_recvfrom_should_fail = 0;
static int g_recvfrom_fail_errno = EAGAIN;
static int g_fcntl_get_should_fail = 0;
static int g_fcntl_set_should_fail = 0;
static int g_fcntl_fail_errno = EINVAL;

extern "C"
{
    int bind(int sockfd, const struct sockaddr *addr, socklen_t len)
    {
        typedef int (*bind_func_type)(int, const struct sockaddr*, socklen_t);
        static bind_func_type real_bind = NULL;

        if (g_bind_should_fail)
        {
            errno = g_bind_fail_errno;
            g_bind_should_fail = 0;
            return (-1);
        }
        if (real_bind == NULL)
            real_bind = reinterpret_cast<bind_func_type>(dlsym(RTLD_NEXT, "bind"));
        return (real_bind(sockfd, addr, len));
    }

    int listen(int sockfd, int backlog)
    {
        typedef int (*listen_func_type)(int, int);
        static listen_func_type real_listen = NULL;

        if (g_listen_should_fail)
        {
            errno = g_listen_fail_errno;
            g_listen_should_fail = 0;
            return (-1);
        }
        if (real_listen == NULL)
            real_listen = reinterpret_cast<listen_func_type>(dlsym(RTLD_NEXT, "listen"));
        return (real_listen(sockfd, backlog));
    }

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        typedef int (*accept_func_type)(int, struct sockaddr*, socklen_t*);
        static accept_func_type real_accept = NULL;

        if (g_accept_should_fail)
        {
            errno = g_accept_fail_errno;
            g_accept_should_fail = 0;
            return (-1);
        }
        if (real_accept == NULL)
            real_accept = reinterpret_cast<accept_func_type>(dlsym(RTLD_NEXT, "accept"));
        return (real_accept(sockfd, addr, addrlen));
    }

    int socket(int domain, int type, int protocol)
    {
        typedef int (*socket_func_type)(int, int, int);
        static socket_func_type real_socket = NULL;

        if (g_socket_should_fail)
        {
            errno = g_socket_fail_errno;
            g_socket_should_fail = 0;
            return (-1);
        }
        if (real_socket == NULL)
            real_socket = reinterpret_cast<socket_func_type>(dlsym(RTLD_NEXT, "socket"));
        return (real_socket(domain, type, protocol));
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t len)
    {
        typedef int (*connect_func_type)(int, const struct sockaddr*, socklen_t);
        static connect_func_type real_connect = NULL;

        if (g_connect_should_fail)
        {
            errno = g_connect_fail_errno;
            g_connect_should_fail = 0;
            return (-1);
        }
        if (real_connect == NULL)
            real_connect = reinterpret_cast<connect_func_type>(dlsym(RTLD_NEXT, "connect"));
        return (real_connect(sockfd, addr, len));
    }

    ssize_t send(int sockfd, const void *buf, size_t len, int flags)
    {
        typedef ssize_t (*send_func_type)(int, const void*, size_t, int);
        static send_func_type real_send = NULL;

        if (g_send_should_fail)
        {
            errno = g_send_fail_errno;
            g_send_should_fail = 0;
            return (-1);
        }
        if (real_send == NULL)
            real_send = reinterpret_cast<send_func_type>(dlsym(RTLD_NEXT, "send"));
        return (real_send(sockfd, buf, len, flags));
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    {
        typedef ssize_t (*recv_func_type)(int, void*, size_t, int);
        static recv_func_type real_recv = NULL;

        if (g_recv_should_fail)
        {
            errno = g_recv_fail_errno;
            g_recv_should_fail = 0;
            return (-1);
        }
        if (real_recv == NULL)
            real_recv = reinterpret_cast<recv_func_type>(dlsym(RTLD_NEXT, "recv"));
        return (real_recv(sockfd, buf, len, flags));
    }

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen)
    {
        typedef ssize_t (*sendto_func_type)(int, const void*, size_t, int, const struct sockaddr*, socklen_t);
        static sendto_func_type real_sendto = NULL;

        if (g_sendto_should_fail)
        {
            errno = g_sendto_fail_errno;
            g_sendto_should_fail = 0;
            return (-1);
        }
        if (real_sendto == NULL)
            real_sendto = reinterpret_cast<sendto_func_type>(dlsym(RTLD_NEXT, "sendto"));
        return (real_sendto(sockfd, buf, len, flags, dest_addr, addrlen));
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen)
    {
        typedef ssize_t (*recvfrom_func_type)(int, void*, size_t, int, struct sockaddr*, socklen_t*);
        static recvfrom_func_type real_recvfrom = NULL;

        if (g_recvfrom_should_fail)
        {
            errno = g_recvfrom_fail_errno;
            g_recvfrom_should_fail = 0;
            return (-1);
        }
        if (real_recvfrom == NULL)
            real_recvfrom = reinterpret_cast<recvfrom_func_type>(dlsym(RTLD_NEXT, "recvfrom"));
        return (real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen));
    }

    int fcntl(int fd, int cmd, ...)
    {
        typedef int (*fcntl_func_type)(int, int, ...);
        static fcntl_func_type real_fcntl = NULL;
        long argument;

        argument = 0;
        if (cmd == F_SETFL)
        {
            va_list args;

            va_start(args, cmd);
            argument = va_arg(args, long);
            va_end(args);
        }
        if (g_fcntl_get_should_fail && cmd == F_GETFL)
        {
            errno = g_fcntl_fail_errno;
            g_fcntl_get_should_fail = 0;
            return (-1);
        }
        if (g_fcntl_set_should_fail && cmd == F_SETFL)
        {
            errno = g_fcntl_fail_errno;
            g_fcntl_set_should_fail = 0;
            return (-1);
        }
        if (real_fcntl == NULL)
            real_fcntl = reinterpret_cast<fcntl_func_type>(dlsym(RTLD_NEXT, "fcntl"));
        if (cmd == F_SETFL)
            return (real_fcntl(fd, cmd, static_cast<int>(argument)));
        return (real_fcntl(fd, cmd));
    }
}

FT_TEST(test_nw_bind_failure_sets_ft_errno, "nw_bind failure populates ft_errno")
{
    int socket_fd;
    struct sockaddr_in address;
    int result;

    socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    FT_ASSERT(socket_fd >= 0);
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    g_bind_fail_errno = EACCES;
    g_bind_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_bind(socket_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
    (void)::close(socket_fd);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(EACCES + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_bind_success_clears_ft_errno, "nw_bind success clears ft_errno")
{
    int socket_fd;
    struct sockaddr_in address;
    int result;

    socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    FT_ASSERT(socket_fd >= 0);
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    ft_errno = FT_EINVAL;
    result = nw_bind(socket_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    (void)::close(socket_fd);
    return (1);
}

FT_TEST(test_nw_send_failure_sets_ft_errno, "nw_send failure populates ft_errno")
{
    int sockets[2];
    ssize_t result;

    FT_ASSERT(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    g_send_fail_errno = EPIPE;
    g_send_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_send(sockets[0], "ab", 2, 0);
    (void)::close(sockets[0]);
    (void)::close(sockets[1]);
    FT_ASSERT_EQ(static_cast<ssize_t>(-1), result);
    FT_ASSERT_EQ(EPIPE + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_send_success_clears_ft_errno, "nw_send success clears ft_errno")
{
    int sockets[2];
    const char *message;
    ssize_t result;
    char buffer[4];

    FT_ASSERT(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    message = "ok";
    ft_errno = FT_EINVAL;
    result = nw_send(sockets[0], message, std::strlen(message), 0);
    FT_ASSERT_EQ(static_cast<ssize_t>(std::strlen(message)), result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    result = ::recv(sockets[1], buffer, sizeof(buffer), 0);
    FT_ASSERT(result >= 0);
    (void)::close(sockets[0]);
    (void)::close(sockets[1]);
    return (1);
}

FT_TEST(test_nw_recv_failure_sets_ft_errno, "nw_recv failure populates ft_errno")
{
    int sockets[2];
    ssize_t result;
    char buffer[4];

    FT_ASSERT(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    g_recv_fail_errno = ECONNRESET;
    g_recv_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_recv(sockets[0], buffer, sizeof(buffer), 0);
    (void)::close(sockets[0]);
    (void)::close(sockets[1]);
    FT_ASSERT_EQ(static_cast<ssize_t>(-1), result);
    FT_ASSERT_EQ(ECONNRESET + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_set_nonblocking_failure_sets_ft_errno, "nw_set_nonblocking failure populates ft_errno")
{
    int sockets[2];
    int result;

    FT_ASSERT(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    g_fcntl_fail_errno = EINVAL;
    g_fcntl_get_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_set_nonblocking(sockets[0]);
    (void)::close(sockets[0]);
    (void)::close(sockets[1]);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(EINVAL + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_set_nonblocking_success_clears_ft_errno, "nw_set_nonblocking success clears ft_errno")
{
    int sockets[2];
    int result;

    FT_ASSERT(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    ft_errno = FT_EINVAL;
    result = nw_set_nonblocking(sockets[0]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    (void)::close(sockets[0]);
    (void)::close(sockets[1]);
    return (1);
}

FT_TEST(test_nw_ssl_write_failure_sets_ft_errno, "nw_ssl_write failure populates ft_errno")
{
    ssize_t result;

    g_mock_ssl_write_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_ssl_write(reinterpret_cast<SSL*>(0x1), "ab", 2);
    FT_ASSERT_EQ(static_cast<ssize_t>(-1), result);
    FT_ASSERT_EQ(SSL_ERROR_SYSCALL + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_ssl_write_success_clears_ft_errno, "nw_ssl_write success clears ft_errno")
{
    ssize_t result;

    ft_errno = FT_EINVAL;
    result = nw_ssl_write(reinterpret_cast<SSL*>(0x1), "ab", 2);
    FT_ASSERT(result >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_nw_ssl_read_failure_sets_ft_errno, "nw_ssl_read failure populates ft_errno")
{
    ssize_t result;
    char buffer[4];

    g_mock_ssl_read_should_fail = 1;
    ft_errno = ER_SUCCESS;
    result = nw_ssl_read(reinterpret_cast<SSL*>(0x1), buffer, sizeof(buffer));
    FT_ASSERT_EQ(static_cast<ssize_t>(-1), result);
    FT_ASSERT_EQ(SSL_ERROR_SYSCALL + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_nw_ssl_read_success_clears_ft_errno, "nw_ssl_read success clears ft_errno")
{
    ssize_t result;
    char buffer[4];

    ft_errno = FT_EINVAL;
    result = nw_ssl_read(reinterpret_cast<SSL*>(0x1), buffer, sizeof(buffer));
    FT_ASSERT(result >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

#else

FT_TEST(test_networking_wrappers_stub, "networking wrapper tests not supported on Windows")
{
    (void)g_mock_ssl_write_should_fail;
    (void)g_mock_ssl_read_should_fail;
    return (1);
}

#endif
