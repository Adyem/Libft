#include "../Networking/socket_class.hpp"
#include "../Networking/networking.hpp"
#include "../Libft/libft.hpp"
#include <cstring>

int test_network_send_receive(void)
{
    SocketConfig server_conf;
    server_conf._port = 54321;
    server_conf._type = SocketType::SERVER;
    ft_socket server(server_conf);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_conf;
    client_conf._port = 54321;
    client_conf._type = SocketType::CLIENT;
    ft_socket client(client_conf);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    int client_fd = nw_accept(server.get_fd(), (struct sockaddr*)&addr, &addr_len);
    if (client_fd < 0)
        return (0);

    const char *msg = "ping";
    if (client.send_all(msg, ft_strlen(msg), 0) != (ssize_t)ft_strlen(msg))
        return (0);
    char buf[16];
    ssize_t r = nw_recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, msg) == 0);
}

int test_network_invalid_ip(void)
{
    SocketConfig conf;
    conf._type = SocketType::SERVER;
    conf._port = 54324;
    conf._ip = "256.0.0.1";
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

int test_network_ipv6_send_receive(void)
{
    SocketConfig server_configuration;
    server_configuration._port = 54325;
    server_configuration._type = SocketType::SERVER;
    server_configuration._address_family = AF_INET6;
    server_configuration._ip = "::1";
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54325;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._address_family = AF_INET6;
    client_configuration._ip = "::1";
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage address;
    socklen_t address_length = sizeof(address);
    int client_file_descriptor = nw_accept(server.get_fd(),
                                           reinterpret_cast<struct sockaddr*>(&address),
                                           &address_length);
    if (client_file_descriptor < 0)
        return (0);

    const char *message = "pong";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

int test_network_ipv6_invalid_ip(void)
{
    SocketConfig configuration;
    configuration._type = SocketType::SERVER;
    configuration._address_family = AF_INET6;
    configuration._port = 54326;
    configuration._ip = "gggg::1";
    ft_socket server(configuration);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

