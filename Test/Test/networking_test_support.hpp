#ifndef NETWORKING_TEST_SUPPORT_HPP
# define NETWORKING_TEST_SUPPORT_HPP

#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Networking/networking.hpp"
#include <cerrno>

static inline ft_bool networking_test_local_ipv4_available(void)
{
    sockaddr_in server_address;
    sockaddr_in client_address;
    socklen_t address_length;
    int32_t server_fd;
    int32_t client_fd;
    int32_t accepted_fd;
    char received_byte;
    ssize_t send_result;
    ssize_t receive_result;

    errno = 0;
    server_fd = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return (FT_FALSE);
    ft_memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = 0;
    if (nw_bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            sizeof(server_address)) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    if (nw_listen(server_fd, 1) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    address_length = sizeof(server_address);
    if (getsockname(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            &address_length) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    client_fd = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    ft_memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client_address.sin_port = server_address.sin_port;
    if (nw_connect(client_fd, reinterpret_cast<struct sockaddr *>(&client_address),
            sizeof(client_address)) != 0)
    {
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    address_length = sizeof(server_address);
    accepted_fd = nw_accept(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            &address_length);
    if (accepted_fd < 0)
    {
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    send_result = nw_send(client_fd, "x", 1, 0);
    if (send_result != 1)
    {
        (void)nw_close(accepted_fd);
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    received_byte = '\0';
    receive_result = nw_recv(accepted_fd, &received_byte, 1, 0);
    if (receive_result != 1 || received_byte != 'x')
    {
        (void)nw_close(accepted_fd);
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    (void)nw_close(accepted_fd);
    (void)nw_close(client_fd);
    (void)nw_close(server_fd);
    return (FT_TRUE);
}

static inline ft_bool networking_test_local_ipv6_available(void)
{
    sockaddr_in6 server_address;
    sockaddr_in6 client_address;
    socklen_t address_length;
    int32_t server_fd;
    int32_t client_fd;
    int32_t accepted_fd;
    char received_byte;
    ssize_t send_result;
    ssize_t receive_result;

    errno = 0;
    server_fd = nw_socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd < 0)
        return (FT_FALSE);
    ft_memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_loopback;
    server_address.sin6_port = 0;
    if (nw_bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            sizeof(server_address)) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    if (nw_listen(server_fd, 1) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    address_length = sizeof(server_address);
    if (getsockname(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            &address_length) != 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    client_fd = nw_socket(AF_INET6, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    ft_memset(&client_address, 0, sizeof(client_address));
    client_address.sin6_family = AF_INET6;
    client_address.sin6_addr = in6addr_loopback;
    client_address.sin6_port = server_address.sin6_port;
    if (nw_connect(client_fd, reinterpret_cast<struct sockaddr *>(&client_address),
            sizeof(client_address)) != 0)
    {
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    address_length = sizeof(server_address);
    accepted_fd = nw_accept(server_fd, reinterpret_cast<struct sockaddr *>(&server_address),
            &address_length);
    if (accepted_fd < 0)
    {
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    send_result = nw_send(client_fd, "x", 1, 0);
    if (send_result != 1)
    {
        (void)nw_close(accepted_fd);
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    received_byte = '\0';
    receive_result = nw_recv(accepted_fd, &received_byte, 1, 0);
    if (receive_result != 1 || received_byte != 'x')
    {
        (void)nw_close(accepted_fd);
        (void)nw_close(client_fd);
        (void)nw_close(server_fd);
        return (FT_FALSE);
    }
    (void)nw_close(accepted_fd);
    (void)nw_close(client_fd);
    (void)nw_close(server_fd);
    return (FT_TRUE);
}

#endif
