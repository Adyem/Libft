#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Networking/http_client.hpp"
#include "../../Networking/ssl_wrapper.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstring>
#include <cstdio>
#include <thread>
#include <chrono>
#include <cerrno>
#include <climits>
#ifndef _WIN32
# include <netdb.h>
#endif

static int g_mock_ssl_write_call_count = 0;
static int g_mock_ssl_write_last_length = 0;
static int g_mock_ssl_read_call_count = 0;
static int g_mock_ssl_read_last_length = 0;
static int g_send_stub_call_count = 0;

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

static ssize_t send_returns_zero_then_error(int socket_fd, const void *buffer,
                                            size_t length, int flags)
{
    (void)socket_fd;
    (void)buffer;
    (void)length;
    (void)flags;
    g_send_stub_call_count++;
    if (g_send_stub_call_count < 3)
        return (0);
#ifdef EPIPE
    errno = EPIPE;
#else
    errno = ECONNRESET;
#endif
    return (-1);
}

static size_t g_http_client_plain_partial_calls = 0;
static int g_http_client_plain_partial_active = 0;

static ssize_t send_partial_http_request_stub(int socket_fd, const void *buffer,
                                             size_t length, int flags)
{
    const char *char_buffer;

    (void)socket_fd;
    (void)flags;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 4 && ft_strncmp(char_buffer, "GET ", 4) == 0)
        g_http_client_plain_partial_active = 1;
    if (g_http_client_plain_partial_active != 0)
    {
        g_http_client_plain_partial_calls++;
        if (length > 6)
            return (6);
        g_http_client_plain_partial_active = 0;
        return (static_cast<ssize_t>(length));
    }
    return (static_cast<ssize_t>(length));
}

static ssize_t send_plain_short_write_stub(int socket_fd, const void *buffer,
                                           size_t length, int flags)
{
    const char *char_buffer;

    (void)socket_fd;
    (void)flags;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 4 && ft_strncmp(char_buffer, "GET ", 4) == 0)
        return (0);
    return (static_cast<ssize_t>(length));
}

static size_t g_http_client_ssl_partial_calls = 0;
static int g_http_client_ssl_partial_active = 0;

static ssize_t ssl_partial_write_stub(SSL *ssl, const void *buffer, size_t length)
{
    const char *char_buffer;

    (void)ssl;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 3 && ft_strncmp(char_buffer, "GET", 3) == 0)
        g_http_client_ssl_partial_active = 1;
    if (g_http_client_ssl_partial_active != 0)
    {
        g_http_client_ssl_partial_calls++;
        if (length > 7)
            return (7);
        g_http_client_ssl_partial_active = 0;
        return (static_cast<ssize_t>(length));
    }
    return (static_cast<ssize_t>(length));
}

static ssize_t ssl_short_write_stub(SSL *ssl, const void *buffer, size_t length)
{
    const char *char_buffer;

    (void)ssl;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 3 && ft_strncmp(char_buffer, "GET", 3) == 0)
        return (0);
    return (static_cast<ssize_t>(length));
}

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
    udp_socket server;
    if (server.initialize(server_configuration) != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54329;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    udp_socket client;
    if (client.initialize(client_configuration) != ER_SUCCESS)
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

FT_TEST(test_network_send_all_peer_close, "send_all handles zero-byte send")
{
    g_send_stub_call_count = 0;
    SocketConfig server_configuration;
    server_configuration._port = 54328;
    server_configuration._type = SocketType::SERVER;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54328;
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

    nw_set_send_stub(&send_returns_zero_then_error);
    const char *message = "halt";
    errno = 0;
    ssize_t result = cmp_socket_send_all(&client, message, ft_strlen(message), 0);
    nw_set_send_stub(NULL);
    FT_CLOSE_SOCKET(client_file_descriptor);

    if (result >= 0)
        return (0);
    if (client.get_error() != SOCKET_SEND_FAILED)
        return (0);
    if (g_send_stub_call_count != 1)
        return (0);
    return (1);
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

struct http_test_server_context
{
    int server_fd;
    bool success;
};

static void http_test_server_run(http_test_server_context *context)
{
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int client_socket;
    char buffer[1024];
    ssize_t bytes_received;
    const char *response_message;
    ssize_t send_result;
    int attempt;

    context->success = false;
    attempt = 0;
    client_socket = -1;
    while (attempt < 200)
    {
        address_length = sizeof(client_address);
        client_socket = nw_accept(context->server_fd,
                                  reinterpret_cast<struct sockaddr*>(&client_address),
                                  &address_length);
        if (client_socket >= 0)
            break;
#ifdef _WIN32
        int error_code = WSAGetLastError();
        if (error_code != WSAEWOULDBLOCK && error_code != WSAEINPROGRESS)
            return ;
#else
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            return ;
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        attempt++;
    }
    if (client_socket < 0)
        return ;
    bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        FT_CLOSE_SOCKET(client_socket);
        return ;
    }
    buffer[bytes_received] = '\0';
    response_message = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPASS";
    send_result = nw_send(client_socket, response_message, ft_strlen(response_message), 0);
    FT_CLOSE_SOCKET(client_socket);
    if (send_result <= 0)
        return ;
    context->success = true;
    return ;
}

FT_TEST(test_http_client_address_fallback, "http client retries multiple addresses")
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
    bool has_ipv4;
    bool has_ipv6;
    int first_family;
    int server_family;
    const char *server_ip;
    SocketConfig server_configuration;
    http_test_server_context server_context;
    ft_string response;
    const struct sockaddr_in *ipv4_address;
    const struct sockaddr_in6 *ipv6_address;
    struct sockaddr_storage local_address;
    socklen_t local_length;
    int formatted_port;
    char port_string[16];
    int client_result;

    std::memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo("localhost", "80", &address_hints, &address_info) != 0)
        return (0);
    has_ipv4 = false;
    has_ipv6 = false;
    first_family = address_info->ai_family;
    current_info = address_info;
    while (current_info != NULL)
    {
        if (current_info->ai_family == AF_INET)
            has_ipv4 = true;
        else if (current_info->ai_family == AF_INET6)
            has_ipv6 = true;
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (!has_ipv4 || !has_ipv6)
        return (1);
    if (first_family == AF_INET6 && has_ipv4)
    {
        server_family = AF_INET;
        server_ip = "127.0.0.1";
    }
    else if (first_family == AF_INET && has_ipv6)
    {
        server_family = AF_INET6;
        server_ip = "::1";
    }
    else
        return (1);
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = server_ip;
    server_configuration._port = 0;
    server_configuration._address_family = server_family;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);
    std::memset(&local_address, 0, sizeof(local_address));
    local_length = sizeof(local_address);
    if (getsockname(server.get_fd(), reinterpret_cast<struct sockaddr*>(&local_address), &local_length) != 0)
    {
        server.close_socket();
        return (0);
    }
    std::memset(port_string, 0, sizeof(port_string));
    if (local_address.ss_family == AF_INET)
    {
        ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&local_address);
        formatted_port = std::snprintf(port_string, sizeof(port_string), "%u", static_cast<unsigned int>(ntohs(ipv4_address->sin_port)));
        if (formatted_port <= 0)
        {
            server.close_socket();
            return (0);
        }
    }
    else if (local_address.ss_family == AF_INET6)
    {
        ipv6_address = reinterpret_cast<const struct sockaddr_in6*>(&local_address);
        formatted_port = std::snprintf(port_string, sizeof(port_string), "%u", static_cast<unsigned int>(ntohs(ipv6_address->sin6_port)));
        if (formatted_port <= 0)
        {
            server.close_socket();
            return (0);
        }
    }
    else
    {
        server.close_socket();
        return (0);
    }
    if (nw_set_nonblocking(server.get_fd()) != 0)
    {
        server.close_socket();
        return (0);
    }
    server_context.server_fd = server.get_fd();
    server_context.success = false;
    std::thread server_thread(http_test_server_run, &server_context);
    response.clear();
    client_result = http_get("localhost", "/", response, false, port_string);
    server_thread.join();
    server.close_socket();
    if (!server_context.success)
        return (0);
    if (client_result != 0)
        return (0);
    return (response == "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPASS");
}

FT_TEST(test_http_client_plain_partial_retry, "http client retries partial nw_send")
{
    const char *request_string;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    g_http_client_plain_partial_calls = 0;
    g_http_client_plain_partial_active = 0;
    nw_set_send_stub(&send_partial_http_request_stub);
    if (http_client_send_plain_request(0, request_string, ft_strlen(request_string)) != 0)
    {
        nw_set_send_stub(NULL);
        return (0);
    }
    nw_set_send_stub(NULL);
    return (g_http_client_plain_partial_calls >= 2);
}

FT_TEST(test_http_client_plain_short_write_sets_errno, "http client detects short write")
{
    const char *request_string;
    int result;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    ft_errno = ER_SUCCESS;
    nw_set_send_stub(&send_plain_short_write_stub);
    result = http_client_send_plain_request(0, request_string, ft_strlen(request_string));
    nw_set_send_stub(NULL);
    if (result != -1)
        return (0);
    return (ft_errno == SOCKET_SEND_FAILED);
}

FT_TEST(test_http_client_ssl_partial_retry, "http client retries partial SSL write")
{
    const char *request_string;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    g_http_client_ssl_partial_calls = 0;
    g_http_client_ssl_partial_active = 0;
    nw_set_ssl_write_stub(&ssl_partial_write_stub);
    if (http_client_send_ssl_request(reinterpret_cast<SSL *>(0x1), request_string, ft_strlen(request_string)) != 0)
    {
        nw_set_ssl_write_stub(NULL);
        return (0);
    }
    nw_set_ssl_write_stub(NULL);
    return (g_http_client_ssl_partial_calls >= 2);
}

FT_TEST(test_http_client_ssl_short_write_sets_errno, "http client SSL detects short write")
{
    const char *request_string;
    int result;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    ft_errno = ER_SUCCESS;
    nw_set_ssl_write_stub(&ssl_short_write_stub);
    result = http_client_send_ssl_request(reinterpret_cast<SSL *>(0x1), request_string, ft_strlen(request_string));
    nw_set_ssl_write_stub(NULL);
    if (result != -1)
        return (0);
    return (ft_errno == SOCKET_SEND_FAILED);
}

