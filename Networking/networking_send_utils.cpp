#include "networking_send_utils.hpp"
#include "networking.hpp"
#include "socket_class.hpp"
#include "../Errno/errno.hpp"
#include <thread>
#include <chrono>
#include <cerrno>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
#endif

int networking_check_socket_after_send(int socket_fd)
{
    int attempt_count;
    int attempt_limit;
    bool disconnect_detected;

    if (socket_fd < 0)
        return (0);
    attempt_limit = 3;
    attempt_count = 0;
    disconnect_detected = false;
    while (attempt_count < attempt_limit)
    {
        int poll_descriptor;
        int poll_result;
        char peek_buffer;
        ssize_t recv_result;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        poll_descriptor = socket_fd;
        poll_result = nw_poll(&poll_descriptor, 1, NULL, 0, 0);
        if (poll_result < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = last_error + ERRNO_OFFSET;
#else
            if (errno == EINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = errno + ERRNO_OFFSET;
#endif
            return (-1);
        }
        if (poll_result == 0 || poll_descriptor == -1)
        {
            attempt_count++;
            continue ;
        }
        peek_buffer = 0;
        recv_result = nw_recv(socket_fd, &peek_buffer, 1, MSG_PEEK);
        if (recv_result == 0)
        {
            disconnect_detected = true;
            break;
        }
        if (recv_result < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEWOULDBLOCK || last_error == WSAEINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = last_error + ERRNO_OFFSET;
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = errno + ERRNO_OFFSET;
#endif
            return (-1);
        }
        break;
    }
    int socket_error;
#ifdef _WIN32
    int option_length;

    socket_error = 0;
    option_length = sizeof(socket_error);
    if (getsockopt(static_cast<SOCKET>(socket_fd),
                   SOL_SOCKET,
                   SO_ERROR,
                   reinterpret_cast<char*>(&socket_error),
                   &option_length) == SOCKET_ERROR)
    {
        ft_errno = WSAGetLastError() + ERRNO_OFFSET;
        return (-1);
    }
#else
    socklen_t option_length;

    socket_error = 0;
    option_length = sizeof(socket_error);
    if (getsockopt(socket_fd,
                   SOL_SOCKET,
                   SO_ERROR,
                   &socket_error,
                   &option_length) < 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
#endif
    if (socket_error != 0)
    {
        ft_errno = socket_error + ERRNO_OFFSET;
        return (-1);
    }
    if (disconnect_detected)
    {
        ft_errno = SOCKET_SEND_FAILED;
        return (-1);
    }
    return (0);
}

int networking_check_ssl_after_send(SSL *ssl_connection)
{
    int attempt_count;
    int attempt_limit;
    int socket_fd;

    if (ssl_connection == NULL)
    {
        ft_errno = SOCKET_SEND_FAILED;
        return (-1);
    }
    attempt_limit = 3;
    attempt_count = 0;
    socket_fd = SSL_get_fd(ssl_connection);
    if (socket_fd < 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    while (attempt_count < attempt_limit)
    {
        char peek_buffer;
        int peek_result;
        int ssl_error;
        int poll_descriptor;
        int poll_result;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        poll_descriptor = socket_fd;
        poll_result = nw_poll(&poll_descriptor, 1, NULL, 0, 0);
        if (poll_result < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = last_error + ERRNO_OFFSET;
#else
            if (errno == EINTR)
            {
                attempt_count++;
                continue ;
            }
            ft_errno = errno + ERRNO_OFFSET;
#endif
            return (-1);
        }
        if (poll_result == 0 || poll_descriptor == -1)
        {
            attempt_count++;
            continue ;
        }
        peek_buffer = 0;
        peek_result = SSL_peek(ssl_connection, &peek_buffer, 1);
        if (peek_result > 0)
            break;
        if (peek_result == 0)
        {
            ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
        ssl_error = SSL_get_error(ssl_connection, peek_result);
        if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
        {
            attempt_count++;
            continue ;
        }
        if (ssl_error == SSL_ERROR_ZERO_RETURN)
        {
            ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
#ifdef _WIN32
        if (ssl_error == SSL_ERROR_SYSCALL)
        {
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEWOULDBLOCK || last_error == WSAEINTR)
            {
                attempt_count++;
                continue ;
            }
            if (last_error != 0)
                ft_errno = last_error + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
#else
        if (ssl_error == SSL_ERROR_SYSCALL)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                attempt_count++;
                continue ;
            }
            if (errno != 0)
                ft_errno = errno + ERRNO_OFFSET;
            else
                ft_errno = SOCKET_SEND_FAILED;
            return (-1);
        }
#endif
        ft_errno = SOCKET_SEND_FAILED;
        return (-1);
    }
    if (networking_check_socket_after_send(socket_fd) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}
