#include "../Networking/socket_class.hpp"
#include "../Networking/networking.hpp"
#include "../Libft/libft.hpp"
#include <cstring>

int test_network_send_receive(void)
{
    SocketConfig server_conf;
    server_conf.port = 54321;
    server_conf.type = SocketType::SERVER;
    ft_socket server(server_conf);
    if (server.get_error() != ER_SUCCESS)
        return 0;

    SocketConfig client_conf;
    client_conf.port = 54321;
    client_conf.type = SocketType::CLIENT;
    ft_socket client(client_conf);
    if (client.get_error() != ER_SUCCESS)
        return 0;

    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    int client_fd = nw_accept(server.get_fd(), (struct sockaddr*)&addr, &addr_len);
    if (client_fd < 0)
        return 0;

    const char *msg = "ping";
    if (client.send_all(msg, ft_strlen(msg), 0) != (ssize_t)ft_strlen(msg))
        return 0;
    char buf[16];
    ssize_t r = nw_recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return ft_strcmp(buf, msg) == 0;
}

int test_network_invalid_ip(void)
{
    SocketConfig conf;
    conf.type = SocketType::SERVER;
    conf.port = 54324;
    conf.ip = "256.0.0.1";
    ft_socket server(conf);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

int test_network_send_uninitialized(void)
{
    ft_socket sock;
    const char *msg = "fail";
    ssize_t r = sock.send_all(msg, ft_strlen(msg), 0);
    return (r < 0 && sock.get_error() == SOCKET_INVALID_CONFIGURATION);
}

