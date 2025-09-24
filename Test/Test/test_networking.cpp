#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/networking_send_utils.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Networking/ssl_wrapper.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <openssl/ssl.h>
#include <cstring>
#include <cerrno>
#include <climits>
#include <unistd.h>

static int g_mock_ssl_write_call_count = 0;
static int g_mock_ssl_write_last_length = 0;
static int g_mock_ssl_read_call_count = 0;
static int g_mock_ssl_read_last_length = 0;

extern "C"
{
    int SSL_write(SSL *ssl, const void *buffer, int length)
    {
        (void)ssl;
        (void)buffer;
        g_mock_ssl_write_call_count++;
        g_mock_ssl_write_last_length = length;
        return (length);
    }

    int SSL_read(SSL *ssl, void *buffer, int length)
    {
        (void)ssl;
        (void)buffer;
        g_mock_ssl_read_call_count++;
        g_mock_ssl_read_last_length = length;
        return (length);
    }
}

FT_TEST(test_ssl_write_rejects_oversize_length, "nw_ssl_write rejects oversize length")
{
    size_t oversize_length;
    ssize_t result;

    g_mock_ssl_write_call_count = 0;
    g_mock_ssl_write_last_length = 0;
    oversize_length = static_cast<size_t>(INT_MAX);
    oversize_length = oversize_length + 1;
    result = nw_ssl_write(reinterpret_cast<SSL *>(0x1), "data", oversize_length);
    if (result >= 0)
        return (0);
    if (g_mock_ssl_write_call_count != 0)
        return (0);
    return (1);
}

FT_TEST(test_ssl_read_rejects_oversize_length, "nw_ssl_read rejects oversize length")
{
    size_t oversize_length;
    ssize_t result;
    char buffer[4];

    g_mock_ssl_read_call_count = 0;
    g_mock_ssl_read_last_length = 0;
    oversize_length = static_cast<size_t>(INT_MAX);
    oversize_length = oversize_length + 1;
    result = nw_ssl_read(reinterpret_cast<SSL *>(0x1), buffer, oversize_length);
    if (result >= 0)
        return (0);
    if (g_mock_ssl_read_call_count != 0)
        return (0);
    return (1);
}

FT_TEST(test_network_send_receive_ipv4, "nw_send/nw_recv IPv4")
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    SocketConfig client_configuration;
    ft_socket client_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *message;
    ssize_t send_result;
    char buffer[16];
    ssize_t bytes_received;

    server_configuration._type = SocketType::SERVER;
    server_configuration._port = 54340;
    server_configuration._ip = "127.0.0.1";
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54340;
    client_configuration._ip = "127.0.0.1";
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
        return (0);
    message = "ping";
    send_result = client_socket.send_all(message, ft_strlen(message), 0);
    if (send_result != static_cast<ssize_t>(ft_strlen(message)))
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    buffer[bytes_received] = '\0';
    FT_CLOSE_SOCKET(client_fd);
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_udp_send_receive_ipv4, "nw_sendto/nw_recvfrom IPv4")
{
    SocketConfig server_configuration;
    udp_socket server;
    SocketConfig client_configuration;
    udp_socket client;
    struct sockaddr_storage destination_address;
    const char *message;
    ssize_t send_result;
    char buffer[16];
    socklen_t address_length;
    ssize_t receive_result;

    server_configuration._type = SocketType::SERVER;
    server_configuration._protocol = IPPROTO_UDP;
    server_configuration._port = 54341;
    if (server.initialize(server_configuration) != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    client_configuration._port = 54341;
    if (client.initialize(client_configuration) != ER_SUCCESS)
        return (0);
    destination_address = server.get_address();
    message = "data";
    send_result = client.send_to(message, ft_strlen(message), 0,
        reinterpret_cast<const struct sockaddr*>(&destination_address),
        sizeof(struct sockaddr_in));
    if (send_result != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    address_length = sizeof(destination_address);
    receive_result = server.receive_from(buffer, sizeof(buffer) - 1, 0,
        reinterpret_cast<struct sockaddr*>(&destination_address), &address_length);
    if (receive_result < 0)
        return (0);
    buffer[receive_result] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_invalid_ip_address, "invalid IPv4 address returns error")
{
    SocketConfig configuration;
    ft_socket server_socket;

    configuration._type = SocketType::SERVER;
    configuration._ip = "256.1.1.1";
    configuration._port = 54342;
    server_socket = ft_socket(configuration);
    if (server_socket.get_error() == ER_SUCCESS)
        return (0);
    return (1);
}

FT_TEST(test_network_poll_ipv6_ready, "nw_poll detects IPv6 readiness")
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    SocketConfig client_configuration;
    ft_socket client_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *message;
    ssize_t send_result;
    int read_descriptors[1];
    int poll_result;
    char buffer[16];
    ssize_t bytes_received;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "::1";
    server_configuration._address_family = AF_INET6;
    server_configuration._port = 54343;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "::1";
    client_configuration._address_family = AF_INET6;
    client_configuration._port = 54343;
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
        return (0);
    if (nw_set_nonblocking(client_fd) != 0)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    message = "start";
    send_result = client_socket.send_all(message, ft_strlen(message), 0);
    if (send_result != static_cast<ssize_t>(ft_strlen(message)))
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    read_descriptors[0] = client_fd;
    poll_result = nw_poll(read_descriptors, 1, ft_nullptr, 0, 1000);
    if (poll_result != 1)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    if (read_descriptors[0] == -1)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (0);
    }
    buffer[bytes_received] = '\0';
    FT_CLOSE_SOCKET(client_fd);
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_networking_check_socket_after_send_detects_disconnect, "networking_check_socket_after_send detects close")
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    SocketConfig client_configuration;
    ft_socket client_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int accepted_fd;
    int check_result;

    server_configuration._type = SocketType::SERVER;
    server_configuration._port = 54344;
    server_configuration._ip = "127.0.0.1";
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54344;
    client_configuration._ip = "127.0.0.1";
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    accepted_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (accepted_fd < 0)
        return (0);
    client_socket.close_socket();
    usleep(50000);
    check_result = networking_check_socket_after_send(accepted_fd);
    FT_CLOSE_SOCKET(accepted_fd);
    if (check_result == 0)
        return (0);
    if (ft_errno != SOCKET_SEND_FAILED)
        return (0);
    return (1);
}
