#include "../../Networking/networking.hpp"
#include "../../Networking/ssl_wrapper.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <openssl/ssl.h>
#include <cerrno>
#ifndef _WIN32
#include <dlfcn.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

struct mock_ssl_state
{
    int placeholder;
};

static mock_ssl_state g_mock_ssl_state;
static bool g_mock_ssl_active = false;
static int g_mock_ssl_fd_value = -1;
static int g_mock_ssl_peek_sequence[8];
static int g_mock_ssl_error_sequence[8];
static int g_mock_ssl_errno_sequence[8];
static int g_mock_ssl_peek_length = 0;
static int g_mock_ssl_peek_index = 0;
static int g_mock_ssl_error_index = 0;

static void mock_ssl_reset(void)
{
    g_mock_ssl_active = true;
    g_mock_ssl_fd_value = -1;
    g_mock_ssl_peek_length = 0;
    g_mock_ssl_peek_index = 0;
    g_mock_ssl_error_index = 0;
    return ;
}

static void mock_ssl_disable(void)
{
    g_mock_ssl_active = false;
    return ;
}

static SSL *mock_ssl_pointer(void)
{
    return (reinterpret_cast<SSL *>(&g_mock_ssl_state));
}

static void mock_ssl_set_fd(int file_descriptor)
{
    g_mock_ssl_fd_value = file_descriptor;
    return ;
}

static void mock_ssl_add_peek_result(int peek_result, int error_code, int error_number)
{
    if (g_mock_ssl_peek_length >= 8)
        return ;
    g_mock_ssl_peek_sequence[g_mock_ssl_peek_length] = peek_result;
    g_mock_ssl_error_sequence[g_mock_ssl_peek_length] = error_code;
    g_mock_ssl_errno_sequence[g_mock_ssl_peek_length] = error_number;
    g_mock_ssl_peek_length++;
    return ;
}

#ifndef _WIN32
extern "C" int SSL_get_fd(const SSL *ssl_connection)
{
    if (g_mock_ssl_active && ssl_connection == reinterpret_cast<const SSL *>(&g_mock_ssl_state))
        return (g_mock_ssl_fd_value);
    typedef int (*ssl_get_fd_type)(const SSL *);
    static ssl_get_fd_type real_ssl_get_fd = NULL;
    if (!real_ssl_get_fd)
        real_ssl_get_fd = reinterpret_cast<ssl_get_fd_type>(dlsym(RTLD_NEXT, "SSL_get_fd"));
    if (!real_ssl_get_fd)
        return (-1);
    return (real_ssl_get_fd(ssl_connection));
}

extern "C" int SSL_peek(SSL *ssl_connection, void *buffer, int buffer_length)
{
    (void)buffer;
    (void)buffer_length;
    if (g_mock_ssl_active && ssl_connection == reinterpret_cast<SSL *>(&g_mock_ssl_state))
    {
        if (g_mock_ssl_peek_length <= 0)
            return (1);
        int index = g_mock_ssl_peek_index;
        if (index >= g_mock_ssl_peek_length)
            index = g_mock_ssl_peek_length - 1;
        errno = g_mock_ssl_errno_sequence[index];
        g_mock_ssl_peek_index = g_mock_ssl_peek_index + 1;
        return (g_mock_ssl_peek_sequence[index]);
    }
    typedef int (*ssl_peek_type)(SSL *, void *, int);
    static ssl_peek_type real_ssl_peek = NULL;
    if (!real_ssl_peek)
        real_ssl_peek = reinterpret_cast<ssl_peek_type>(dlsym(RTLD_NEXT, "SSL_peek"));
    if (!real_ssl_peek)
        return (-1);
    return (real_ssl_peek(ssl_connection, buffer, buffer_length));
}

extern "C" int SSL_get_error(const SSL *ssl_connection, int return_code)
{
    (void)return_code;
    if (g_mock_ssl_active && ssl_connection == reinterpret_cast<const SSL *>(&g_mock_ssl_state))
    {
        if (g_mock_ssl_peek_length <= 0)
            return (SSL_ERROR_NONE);
        int index = g_mock_ssl_error_index;
        if (index >= g_mock_ssl_peek_length)
            index = g_mock_ssl_peek_length - 1;
        g_mock_ssl_error_index = g_mock_ssl_error_index + 1;
        return (g_mock_ssl_error_sequence[index]);
    }
    typedef int (*ssl_get_error_type)(const SSL *, int);
    static ssl_get_error_type real_ssl_get_error = NULL;
    if (!real_ssl_get_error)
        real_ssl_get_error = reinterpret_cast<ssl_get_error_type>(dlsym(RTLD_NEXT, "SSL_get_error"));
    if (!real_ssl_get_error)
        return (SSL_ERROR_SYSCALL);
    return (real_ssl_get_error(ssl_connection, return_code));
}
#endif

FT_TEST(test_networking_check_ssl_after_send_null_pointer_sets_errno,
    "networking_check_ssl_after_send rejects null SSL pointer")
{
    int result;

    mock_ssl_disable();
    ft_errno = ER_SUCCESS;
    result = networking_check_ssl_after_send(ft_nullptr);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(SOCKET_SEND_FAILED, ft_errno);
    return (1);
}

#ifndef _WIN32
static int create_socket_pair(int sockets[2])
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return (-1);
    return (0);
}

FT_TEST(test_networking_check_ssl_after_send_reports_success_without_fd,
    "networking_check_ssl_after_send treats negative fd as success")
{
    int result;

    mock_ssl_reset();
    mock_ssl_set_fd(-1);
    ft_errno = FT_EINVAL;
    result = networking_check_ssl_after_send(mock_ssl_pointer());
    mock_ssl_disable();
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_networking_check_ssl_after_send_detects_remote_close,
    "networking_check_ssl_after_send reports zero-length SSL_peek")
{
    int sockets[2];
    int result;

    if (create_socket_pair(sockets) != 0)
        return (0);
    mock_ssl_reset();
    mock_ssl_set_fd(sockets[0]);
    mock_ssl_add_peek_result(0, SSL_ERROR_NONE, 0);
    ft_errno = ER_SUCCESS;
    if (::write(sockets[1], "x", 1) != 1)
    {
        ::close(sockets[0]);
        ::close(sockets[1]);
        mock_ssl_disable();
        return (0);
    }
    result = networking_check_ssl_after_send(mock_ssl_pointer());
    ::close(sockets[0]);
    ::close(sockets[1]);
    mock_ssl_disable();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(SOCKET_SEND_FAILED, ft_errno);
    return (1);
}

FT_TEST(test_networking_check_ssl_after_send_retries_want_read,
    "networking_check_ssl_after_send retries SSL_ERROR_WANT_* and succeeds")
{
    int sockets[2];
    int result;

    if (create_socket_pair(sockets) != 0)
        return (0);
    mock_ssl_reset();
    mock_ssl_set_fd(sockets[0]);
    mock_ssl_add_peek_result(-1, SSL_ERROR_WANT_READ, EAGAIN);
    mock_ssl_add_peek_result(-1, SSL_ERROR_WANT_WRITE, EAGAIN);
    mock_ssl_add_peek_result(1, SSL_ERROR_NONE, 0);
    ft_errno = FT_EINVAL;
    if (::write(sockets[1], "y", 1) != 1)
    {
        ::close(sockets[0]);
        ::close(sockets[1]);
        mock_ssl_disable();
        return (0);
    }
    result = networking_check_ssl_after_send(mock_ssl_pointer());
    ::close(sockets[0]);
    ::close(sockets[1]);
    mock_ssl_disable();
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_networking_check_ssl_after_send_propagates_syscall_errno,
    "networking_check_ssl_after_send surfaces SSL_ERROR_SYSCALL errno")
{
    int sockets[2];
    int result;

    if (create_socket_pair(sockets) != 0)
        return (0);
    mock_ssl_reset();
    mock_ssl_set_fd(sockets[0]);
    mock_ssl_add_peek_result(-1, SSL_ERROR_SYSCALL, EPIPE);
    ft_errno = ER_SUCCESS;
    if (::write(sockets[1], "z", 1) != 1)
    {
        ::close(sockets[0]);
        ::close(sockets[1]);
        mock_ssl_disable();
        return (0);
    }
    result = networking_check_ssl_after_send(mock_ssl_pointer());
    ::close(sockets[0]);
    ::close(sockets[1]);
    mock_ssl_disable();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(EPIPE + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_networking_check_ssl_after_send_zero_return_sets_socket_failed,
    "networking_check_ssl_after_send maps SSL_ERROR_ZERO_RETURN to SOCKET_SEND_FAILED")
{
    int sockets[2];
    int result;

    if (create_socket_pair(sockets) != 0)
        return (0);
    mock_ssl_reset();
    mock_ssl_set_fd(sockets[0]);
    mock_ssl_add_peek_result(-1, SSL_ERROR_ZERO_RETURN, 0);
    ft_errno = ER_SUCCESS;
    if (::write(sockets[1], "q", 1) != 1)
    {
        ::close(sockets[0]);
        ::close(sockets[1]);
        mock_ssl_disable();
        return (0);
    }
    result = networking_check_ssl_after_send(mock_ssl_pointer());
    ::close(sockets[0]);
    ::close(sockets[1]);
    mock_ssl_disable();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(SOCKET_SEND_FAILED, ft_errno);
    return (1);
}
#endif
