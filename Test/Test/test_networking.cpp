#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

FT_TEST(test_network_send_receive, "nw_send/nw_recv IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54321;
    server_configuration._type = SocketType::SERVER;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54321;
    client_configuration._type = SocketType::CLIENT;
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

    const char *message = "ping";
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

FT_TEST(test_udp_send_receive, "nw_sendto/nw_recvfrom IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54329;
    server_configuration._type = SocketType::SERVER;
    server_configuration._protocol = IPPROTO_UDP;
    udp_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54329;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    udp_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage dest;
    dest = server.get_address();
    const char *message = "data";
    ssize_t sent = client.send_to(message, ft_strlen(message), 0,
                                  reinterpret_cast<const struct sockaddr*>(&dest),
                                  sizeof(struct sockaddr_in));
    if (sent != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    char buffer[16];
    socklen_t addr_len = sizeof(dest);
    ssize_t received = server.receive_from(buffer, sizeof(buffer) - 1, 0,
                                          reinterpret_cast<struct sockaddr*>(&dest),
                                          &addr_len);
    if (received < 0)
        return (0);
    buffer[received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_invalid_ip, "invalid IPv4 address")
{
    SocketConfig configuration;
    configuration._type = SocketType::SERVER;
    configuration._port = 54324;
    configuration._ip = "256.0.0.1";
    ft_socket server(configuration);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_send_uninitialized, "send on uninitialized socket")
{
    ft_socket socket_object;
    const char *message = "fail";
    ssize_t result = socket_object.send_all(message, ft_strlen(message), 0);
    return (result < 0 && socket_object.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_ipv6_send_receive, "nw_send/nw_recv IPv6")
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

FT_TEST(test_network_ipv6_invalid_ip, "invalid IPv6 address")
{
    SocketConfig configuration;
    configuration._type = SocketType::SERVER;
    configuration._address_family = AF_INET6;
    configuration._port = 54326;
    configuration._ip = "gggg::1";
    ft_socket server(configuration);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_poll_ipv4, "nw_poll IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54327;
    server_configuration._type = SocketType::SERVER;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54327;
    client_configuration._type = SocketType::CLIENT;
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
    if (nw_set_nonblocking(client_file_descriptor) != 0)
        return (0);

    const char *message = "ready";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    int read_descriptors[1];
    read_descriptors[0] = client_file_descriptor;
    int result = nw_poll(read_descriptors, 1, NULL, 0, 1000);
    if (result != 1 || read_descriptors[0] == -1)
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_poll_ipv6, "nw_poll IPv6")
{
    SocketConfig server_configuration;
    server_configuration._port = 54328;
    server_configuration._type = SocketType::SERVER;
    server_configuration._address_family = AF_INET6;
    server_configuration._ip = "::1";
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54328;
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
    if (nw_set_nonblocking(client_file_descriptor) != 0)
        return (0);

    const char *message = "start";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    int read_descriptors[1];
    read_descriptors[0] = client_file_descriptor;
    int result = nw_poll(read_descriptors, 1, NULL, 0, 1000);
    if (result != 1 || read_descriptors[0] == -1)
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

