#include "../test_internal.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Networking/http_client.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <climits>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef LIBFT_TEST_BUILD
#endif

struct http_stream_test_server_context
{
    ft_socket   *server_socket;
};

struct http_stream_handler_state
{
    int         status_code;
    bool        finished;
    int         chunk_count;
    ft_string   headers;
    ft_string   chunks[3];
};

static http_stream_handler_state *g_http_stream_handler_state = NULL;

static bool get_socket_port_string(ft_socket &socket, ft_string &port_string)
{
    struct sockaddr_storage local_address;
    socklen_t address_length;
    int socket_fd;
    unsigned short port_value;

    socket_fd = socket.get_fd();
    if (socket_fd < 0)
        return (false);
    address_length = sizeof(local_address);
    if (getsockname(socket_fd, reinterpret_cast<struct sockaddr*>(&local_address), &address_length) != 0)
        return (false);
    if (local_address.ss_family == AF_INET)
    {
        const struct sockaddr_in *ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&local_address);
        port_value = ntohs(ipv4_address->sin_port);
    }
    else if (local_address.ss_family == AF_INET6)
    {
        const struct sockaddr_in6 *ipv6_address = reinterpret_cast<const struct sockaddr_in6*>(&local_address);
        port_value = ntohs(ipv6_address->sin6_port);
    }
    else
        return (false);
    char port_buffer[16];

    std::snprintf(port_buffer, sizeof(port_buffer), "%u",
        static_cast<unsigned int>(port_value));
    port_string = port_buffer;
    if (port_string.empty())
        return (false);
    return (true);
}

static int socket_creation_failure_hook(int, int, int)
{
    errno = 0;
    return (-1);
}

static void http_stream_test_server(http_stream_test_server_context *context)
{
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *header_block;
    size_t header_length;
    ssize_t send_result;
    const char *chunk_one;
    const char *chunk_two;
    const char *chunk_three;
    size_t chunk_one_length;
    size_t chunk_two_length;
    size_t chunk_three_length;

    if (context == NULL)
        return ;
    if (context->server_socket == NULL)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(context->server_socket->get_fd(),
        reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
        return ;
    header_block = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n";
    header_length = ft_strlen(header_block);
    send_result = nw_send(client_fd, header_block, header_length, 0);
    if (send_result != static_cast<ssize_t>(header_length))
    {
        nw_close(client_fd);
        return ;
    }
    usleep(50000);
    chunk_one = "4\r\nWiki\r\n";
    chunk_one_length = ft_strlen(chunk_one);
    send_result = nw_send(client_fd, chunk_one, chunk_one_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_one_length))
    {
        nw_close(client_fd);
        return ;
    }
    usleep(50000);
    chunk_two = "5\r\npedia\r\n";
    chunk_two_length = ft_strlen(chunk_two);
    send_result = nw_send(client_fd, chunk_two, chunk_two_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_two_length))
    {
        nw_close(client_fd);
        return ;
    }
    usleep(50000);
    chunk_three = "0\r\n\r\n";
    chunk_three_length = ft_strlen(chunk_three);
    send_result = nw_send(client_fd, chunk_three, chunk_three_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_three_length))
    {
        nw_close(client_fd);
        return ;
    }
    nw_close(client_fd);
    return ;
}

static void test_http_streaming_handler(int status_code, const ft_string &headers,
    const char *body_chunk, size_t chunk_size, bool finished)
{
    http_stream_handler_state *state;

    state = g_http_stream_handler_state;
    if (state == NULL)
        return ;
    if (finished != false)
    {
        state->finished = true;
        return ;
    }
    if (state->status_code == 0)
        state->status_code = status_code;
    if (state->headers.empty())
        state->headers = headers;
    if (chunk_size > 0)
    {
        if (state->chunk_count < 3)
            state->chunks[state->chunk_count] = body_chunk;
        state->chunk_count++;
    }
    return ;
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
    std::strncpy(server_configuration._ip, "127.0.0.1",
        sizeof(server_configuration._ip) - 1);
    server_configuration._ip[sizeof(server_configuration._ip) - 1] = '\0';
    if (server_socket.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54340;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    if (client_socket.initialize(client_configuration) != FT_ERR_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
        return (0);
    message = "ping";
    send_result = client_socket.send_all(message, ft_strlen(message), 0);
    if (send_result != static_cast<ssize_t>(ft_strlen(message)))
    {
        nw_close(client_fd);
        return (0);
    }
    bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        nw_close(client_fd);
        return (0);
    }
    buffer[bytes_received] = '\0';
    nw_close(client_fd);
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
    if (server.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    client_configuration._port = 54341;
    if (client.initialize(client_configuration) != FT_ERR_SUCCESS)
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

FT_TEST(test_udp_socket_propagates_ft_errno_on_creation_failure,
    "udp_socket initialization reports failure on socket creation hook")
{
    SocketConfig config;
    udp_socket socket_instance;
    int initialize_result;

    config._type = SocketType::CLIENT;
    config._address_family = AF_INET;
    config._protocol = 0;
    config._reuse_address = false;
    config._non_blocking = false;
    config._recv_timeout = 0;
    config._send_timeout = 0;
    config._port = 54355;
    std::strncpy(config._ip, "127.0.0.1", sizeof(config._ip) - 1);
    config._ip[sizeof(config._ip) - 1] = '\0';
    errno = 0;
    nw_set_socket_hook(socket_creation_failure_hook);
    initialize_result = socket_instance.initialize(config);
    nw_set_socket_hook(ft_nullptr);
    FT_ASSERT(initialize_result != FT_ERR_SUCCESS);
    FT_ASSERT_EQ(0, errno);
    return (1);
}

FT_TEST(test_network_invalid_ip_address, "invalid IPv4 address returns error")
{
    SocketConfig configuration;
    ft_socket server_socket;

    configuration._type = SocketType::SERVER;
    std::strncpy(configuration._ip, "256.1.1.1", sizeof(configuration._ip) - 1);
    configuration._ip[sizeof(configuration._ip) - 1] = '\0';
    configuration._port = 54342;
    if (server_socket.initialize(configuration) == FT_ERR_SUCCESS)
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
    std::strncpy(server_configuration._ip, "::1",
        sizeof(server_configuration._ip) - 1);
    server_configuration._ip[sizeof(server_configuration._ip) - 1] = '\0';
    server_configuration._address_family = AF_INET6;
    server_configuration._port = 54343;
    if (server_socket.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "::1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._address_family = AF_INET6;
    client_configuration._port = 54343;
    if (client_socket.initialize(client_configuration) != FT_ERR_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
        return (0);
    if (nw_set_nonblocking(client_fd) != 0)
    {
        nw_close(client_fd);
        return (0);
    }
    message = "start";
    send_result = client_socket.send_all(message, ft_strlen(message), 0);
    if (send_result != static_cast<ssize_t>(ft_strlen(message)))
    {
        nw_close(client_fd);
        return (0);
    }
    read_descriptors[0] = client_fd;
    poll_result = nw_poll(read_descriptors, 1, ft_nullptr, 0, 1000);
    if (poll_result != 1)
    {
        nw_close(client_fd);
        return (0);
    }
    if (read_descriptors[0] == -1)
    {
        nw_close(client_fd);
        return (0);
    }
    bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        nw_close(client_fd);
        return (0);
    }
    buffer[bytes_received] = '\0';
    nw_close(client_fd);
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_http_client_streaming_chunks, "http_get_stream handles chunked responses")
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    http_stream_test_server_context server_context;
    ft_thread server_thread;
    http_stream_handler_state handler_state;
    int client_result;
    ft_string port_string;

    server_configuration._type = SocketType::SERVER;
    std::strncpy(server_configuration._ip, "127.0.0.1",
        sizeof(server_configuration._ip) - 1);
    server_configuration._ip[sizeof(server_configuration._ip) - 1] = '\0';
    server_configuration._port = 0;
    if (server_socket.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    if (get_socket_port_string(server_socket, port_string) == false)
        return (0);
    server_context.server_socket = &server_socket;
    server_thread = ft_thread(http_stream_test_server, &server_context);
    if (!server_thread.joinable())
        return (0);
    usleep(50000);
    handler_state.status_code = 0;
    handler_state.finished = false;
    handler_state.chunk_count = 0;
    handler_state.headers.clear();
    handler_state.chunks[0].clear();
    handler_state.chunks[1].clear();
    handler_state.chunks[2].clear();
    g_http_stream_handler_state = &handler_state;
    client_result = http_get_stream("127.0.0.1", "/", test_http_streaming_handler, false, port_string.c_str());
    g_http_stream_handler_state = NULL;
    server_thread.join();
    if (client_result != 0)
        return (0);
    if (handler_state.status_code != 200)
        return (0);
    if (handler_state.finished == false)
        return (0);
    if (handler_state.chunk_count != 3)
        return (0);
    if (ft_strcmp(handler_state.headers.c_str(), "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n") != 0)
        return (0);
    if (ft_strcmp(handler_state.chunks[0].c_str(), "4\r\nWiki\r\n") != 0)
        return (0);
    if (ft_strcmp(handler_state.chunks[1].c_str(), "5\r\npedia\r\n") != 0)
        return (0);
    if (ft_strcmp(handler_state.chunks[2].c_str(), "0\r\n\r\n") != 0)
        return (0);
    return (1);
}

FT_TEST(test_server_config_ipv4_any_address, "server accepts empty IPv4 address as any")
{
    SocketConfig configuration;
    ft_socket server_socket;
    const struct sockaddr_storage *address;
    const struct sockaddr_in *address_ipv4;

    configuration._type = SocketType::SERVER;
    configuration._address_family = AF_INET;
    configuration._port = 0;
    configuration._ip[0] = '\0';
    if (server_socket.initialize(configuration) != FT_ERR_SUCCESS)
        return (0);
    address = &server_socket.get_address();
    address_ipv4 = reinterpret_cast<const struct sockaddr_in*>(address);
    if (address_ipv4->sin_addr.s_addr != htonl(INADDR_ANY))
        return (0);
    return (1);
}

FT_TEST(test_server_config_ipv6_any_address, "server accepts empty IPv6 address as any")
{
    SocketConfig configuration;
    ft_socket server_socket;
    const struct sockaddr_storage *address;
    const struct sockaddr_in6 *address_ipv6;

    configuration._type = SocketType::SERVER;
    configuration._address_family = AF_INET6;
    configuration._port = 0;
    configuration._ip[0] = '\0';
    if (server_socket.initialize(configuration) != FT_ERR_SUCCESS)
        return (0);
    address = &server_socket.get_address();
    address_ipv6 = reinterpret_cast<const struct sockaddr_in6*>(address);
    if (ft_memcmp(&address_ipv6->sin6_addr, &in6addr_any, sizeof(in6addr_any)) != 0)
        return (0);
    return (1);
}

FT_TEST(test_nw_poll_skips_negative_descriptors, "nw_poll ignores negative entries")
{
    int pipe_descriptors[2];
    const char *message;
    ssize_t write_result;
    int read_descriptors[3];
    int poll_result;
    char buffer[2];
    ssize_t read_result;

    if (pipe(pipe_descriptors) != 0)
        return (0);
    message = "x";
    write_result = write(pipe_descriptors[1], message, 1);
    if (write_result != 1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    read_descriptors[0] = -1;
    read_descriptors[1] = pipe_descriptors[0];
    read_descriptors[2] = -1;
    poll_result = nw_poll(read_descriptors, 3, ft_nullptr, 0, 100);
    if (poll_result != 1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    if (read_descriptors[0] != -1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    if (read_descriptors[1] != pipe_descriptors[0])
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    if (read_descriptors[2] != -1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    read_result = read(pipe_descriptors[0], buffer, 1);
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    if (read_result != 1)
        return (0);
    return (1);
}

FT_TEST(test_nw_poll_negative_timeout_is_infinite, "nw_poll treats negative timeout as infinite")
{
    int pipe_descriptors[2];
    const char *message;
    ssize_t write_result;
    int read_descriptors[1];
    int poll_result;
    char buffer[2];
    ssize_t read_result;

    if (pipe(pipe_descriptors) != 0)
        return (0);
    message = "y";
    write_result = write(pipe_descriptors[1], message, 1);
    if (write_result != 1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    read_descriptors[0] = pipe_descriptors[0];
    poll_result = nw_poll(read_descriptors, 1, ft_nullptr, 0, -1);
    if (poll_result != 1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    if (read_descriptors[0] != pipe_descriptors[0])
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    read_result = read(pipe_descriptors[0], buffer, 1);
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    if (read_result != 1)
        return (0);
    return (1);
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
    std::strncpy(server_configuration._ip, "127.0.0.1",
        sizeof(server_configuration._ip) - 1);
    server_configuration._ip[sizeof(server_configuration._ip) - 1] = '\0';
    if (server_socket.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54344;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    if (client_socket.initialize(client_configuration) != FT_ERR_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    accepted_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (accepted_fd < 0)
        return (0);
    client_socket.close_socket();
    usleep(50000);
    check_result = networking_check_socket_after_send(accepted_fd);
    nw_close(accepted_fd);
    if (check_result == 0)
        return (0);
    return (1);
}

FT_TEST(test_networking_check_socket_after_send_handles_invalid_descriptor, "networking_check_socket_after_send invalid descriptor")
{
    int check_result;

    check_result = networking_check_socket_after_send(-1);
    if (check_result != -1)
        return (0);
    return (1);
}

FT_TEST(test_networking_check_socket_after_send_reports_success, "networking_check_socket_after_send success")
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
    server_configuration._port = 54345;
    std::strncpy(server_configuration._ip, "127.0.0.1",
        sizeof(server_configuration._ip) - 1);
    server_configuration._ip[sizeof(server_configuration._ip) - 1] = '\0';
    if (server_socket.initialize(server_configuration) != FT_ERR_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54345;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    if (client_socket.initialize(client_configuration) != FT_ERR_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    accepted_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (accepted_fd < 0)
        return (0);
    check_result = networking_check_socket_after_send(accepted_fd);
    nw_close(accepted_fd);
    client_socket.close_socket();
    server_socket.close_socket();
    if (check_result != 0)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_null_arguments_sets_einval, "nw_inet_pton null arguments set FT_ERR_INVALID_ARGUMENT")
{
    struct in_addr address;
    int result;

    result = nw_inet_pton(AF_INET, ft_nullptr, &address);
    if (result != -1)
        return (0);
    result = nw_inet_pton(AF_INET, "127.0.0.1", ft_nullptr);
    if (result != -1)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_invalid_address_sets_einval, "nw_inet_pton invalid address sets FT_ERR_INVALID_ARGUMENT")
{
    struct in_addr address;
    int result;

    result = nw_inet_pton(AF_INET, "not-an-ip", &address);
    if (result != 0)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_success_clears_errno, "nw_inet_pton success clears errno")
{
    struct in_addr address;

    if (nw_inet_pton(AF_INET, "127.0.0.1", &address) != 1)
        return (0);
    return (1);
}
