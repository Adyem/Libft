#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Networking/http_client.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/thread.hpp"
#include <cstring>
#include <cerrno>
#include <climits>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

static int socket_creation_failure_hook(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    errno = 0;
    ft_errno = SOCKET_CREATION_FAILED;
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
        FT_CLOSE_SOCKET(client_fd);
        return ;
    }
    usleep(50000);
    chunk_one = "4\r\nWiki\r\n";
    chunk_one_length = ft_strlen(chunk_one);
    send_result = nw_send(client_fd, chunk_one, chunk_one_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_one_length))
    {
        FT_CLOSE_SOCKET(client_fd);
        return ;
    }
    usleep(50000);
    chunk_two = "5\r\npedia\r\n";
    chunk_two_length = ft_strlen(chunk_two);
    send_result = nw_send(client_fd, chunk_two, chunk_two_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_two_length))
    {
        FT_CLOSE_SOCKET(client_fd);
        return ;
    }
    usleep(50000);
    chunk_three = "0\r\n\r\n";
    chunk_three_length = ft_strlen(chunk_three);
    send_result = nw_send(client_fd, chunk_three, chunk_three_length, 0);
    if (send_result != static_cast<ssize_t>(chunk_three_length))
    {
        FT_CLOSE_SOCKET(client_fd);
        return ;
    }
    FT_CLOSE_SOCKET(client_fd);
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

FT_TEST(test_udp_socket_propagates_ft_errno_on_creation_failure,
    "udp_socket initialization forwards ft_errno")
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
    config._ip = "127.0.0.1";
    if (config._ip.get_error() != ER_SUCCESS)
        return (0);
    errno = 0;
    ft_errno = ER_SUCCESS;
    nw_set_socket_hook(socket_creation_failure_hook);
    initialize_result = socket_instance.initialize(config);
    nw_set_socket_hook(ft_nullptr);
    FT_ASSERT(initialize_result != ER_SUCCESS);
    FT_ASSERT_EQ(SOCKET_CREATION_FAILED, socket_instance.get_error());
    FT_ASSERT_EQ(SOCKET_CREATION_FAILED, ft_errno);
    FT_ASSERT_EQ(0, errno);
    return (0);
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

FT_TEST(test_http_client_streaming_chunks, "http_get_stream handles chunked responses")
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    http_stream_test_server_context server_context;
    ft_thread server_thread;
    http_stream_handler_state handler_state;
    int client_result;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54360;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return (0);
    server_context.server_socket = &server_socket;
    server_thread = ft_thread(http_stream_test_server, &server_context);
    if (server_thread.get_error() != ER_SUCCESS)
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
    client_result = http_get_stream("127.0.0.1", "/", test_http_streaming_handler, false, "54360");
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
    configuration._port = 54350;
    configuration._ip = "";
    server_socket = ft_socket(configuration);
    if (server_socket.get_error() != ER_SUCCESS)
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
    configuration._port = 54351;
    configuration._ip = "";
    server_socket = ft_socket(configuration);
    if (server_socket.get_error() != ER_SUCCESS)
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

FT_TEST(test_networking_check_socket_after_send_handles_invalid_descriptor, "networking_check_socket_after_send invalid descriptor")
{
    int check_result;

    ft_errno = ER_SUCCESS;
    check_result = networking_check_socket_after_send(-1);
    if (check_result != -1)
        return (0);
    if (ft_errno != FT_EINVAL)
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
    server_configuration._ip = "127.0.0.1";
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54345;
    client_configuration._ip = "127.0.0.1";
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
        return (0);
    address_length = sizeof(address_storage);
    accepted_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (accepted_fd < 0)
        return (0);
    ft_errno = SOCKET_SEND_FAILED;
    check_result = networking_check_socket_after_send(accepted_fd);
    FT_CLOSE_SOCKET(accepted_fd);
    client_socket.close_socket();
    server_socket.close_socket();
    if (check_result != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_null_arguments_sets_einval, "nw_inet_pton null arguments set FT_EINVAL")
{
    struct in_addr address;
    int result;

    result = nw_inet_pton(AF_INET, ft_nullptr, &address);
    if (result != -1)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    result = nw_inet_pton(AF_INET, "127.0.0.1", ft_nullptr);
    if (result != -1)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_invalid_address_sets_einval, "nw_inet_pton invalid address sets FT_EINVAL")
{
    struct in_addr address;
    int result;

    ft_errno = ER_SUCCESS;
    result = nw_inet_pton(AF_INET, "not-an-ip", &address);
    if (result != 0)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_nw_inet_pton_success_clears_errno, "nw_inet_pton success clears errno")
{
    struct in_addr address;

    ft_errno = FT_EINVAL;
    if (nw_inet_pton(AF_INET, "127.0.0.1", &address) != 1)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    return (1);
}
