#include "../../Networking/http_server.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include "../../Errno/errno.hpp"
#include <unistd.h>

static size_t g_http_server_partial_calls = 0;
static size_t g_http_server_partial_total = 0;
static size_t g_http_server_expected_total = 0;
static int g_http_server_error_stub_called = 0;
static int g_http_server_partial_active = 0;

static ssize_t http_server_partial_send_stub(int socket_fd, const void *buffer,
                                            size_t length, int flags)
{
    const char *char_buffer;
    size_t chunk_size;
    ssize_t send_result;

    (void)socket_fd;
    (void)flags;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 4 && ft_strncmp(char_buffer, "HTTP", 4) == 0)
        g_http_server_partial_active = 1;
    if (g_http_server_partial_active != 0)
    {
        g_http_server_partial_calls++;
        if (length > 7)
            chunk_size = 7;
        else
            chunk_size = length;
        nw_set_send_stub(NULL);
        send_result = nw_send(socket_fd, buffer, chunk_size, flags);
        nw_set_send_stub(&http_server_partial_send_stub);
        if (send_result > 0)
            g_http_server_partial_total += static_cast<size_t>(send_result);
        if (length <= 7)
            g_http_server_partial_active = 0;
        return (send_result);
    }
    nw_set_send_stub(NULL);
    send_result = nw_send(socket_fd, buffer, length, flags);
    nw_set_send_stub(&http_server_partial_send_stub);
    return (send_result);
}

static ssize_t http_server_short_write_stub(int socket_fd, const void *buffer,
                                            size_t length, int flags)
{
    const char *char_buffer;
    ssize_t send_result;

    (void)socket_fd;
    (void)flags;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 4 && ft_strncmp(char_buffer, "HTTP", 4) == 0)
    {
        g_http_server_error_stub_called = 1;
        nw_set_send_stub(NULL);
        return (0);
    }
    nw_set_send_stub(NULL);
    send_result = nw_send(socket_fd, buffer, length, flags);
    nw_set_send_stub(&http_server_short_write_stub);
    return (send_result);
}

static void server_worker(ft_http_server *server)
{
    server->run_once();
    return ;
}

FT_TEST(test_http_server_get, "HTTP server GET")
{
    ft_http_server server;

    if (server.start("127.0.0.1", 54330) != 0)
        return (0);
    ft_thread server_thread_object(server_worker, &server);
    SocketConfig client_configuration;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54330;
    ft_socket client_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread_object.join();
        return (0);
    }
    const char *request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    client_socket.send_all(request_string, ft_strlen(request_string), 0);
    char buffer[256];
    ssize_t bytes_received = client_socket.receive_data(buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        server_thread_object.join();
        return (0);
    }
    buffer[bytes_received] = '\0';
    server_thread_object.join();
    return (ft_strnstr(buffer, "GET", bytes_received) != ft_nullptr);
}

FT_TEST(test_http_server_post, "HTTP server POST")
{
    ft_http_server server;

    if (server.start("127.0.0.1", 54331) != 0)
        return (0);
    ft_thread server_thread_object(server_worker, &server);
    SocketConfig client_configuration;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54331;
    ft_socket client_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread_object.join();
        return (0);
    }
    const char *request_headers = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nConnection: close\r\n\r\n";
    const char *body_prefix = "He";
    const char *body_suffix = "llo";
    client_socket.send_all(request_headers, ft_strlen(request_headers), 0);
    usleep(50000);
    client_socket.send_all(body_prefix, ft_strlen(body_prefix), 0);
    usleep(50000);
    client_socket.send_all(body_suffix, ft_strlen(body_suffix), 0);
    char buffer[256];
    ssize_t bytes_received = client_socket.receive_data(buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        server_thread_object.join();
        return (0);
    }
    buffer[bytes_received] = '\0';
    server_thread_object.join();
    return (ft_strnstr(buffer, "Hello", bytes_received) != ft_nullptr);
}

FT_TEST(test_http_server_partial_send_retries, "HTTP server retries partial nw_send")
{
    ft_http_server server;
    const char *expected_response;

    if (server.start("127.0.0.1", 54332) != 0)
        return (0);
    expected_response = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nGET";
    g_http_server_partial_calls = 0;
    g_http_server_partial_total = 0;
    g_http_server_expected_total = ft_strlen(expected_response);
    g_http_server_partial_active = 0;
    nw_set_send_stub(&http_server_partial_send_stub);
    ft_thread server_thread_object(server_worker, &server);
    SocketConfig client_configuration;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54332;
    ft_socket client_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread_object.join();
        nw_set_send_stub(NULL);
        return (0);
    }
    const char *request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    client_socket.send_all(request_string, ft_strlen(request_string), 0);
    server_thread_object.join();
    nw_set_send_stub(NULL);
    if (server.get_error() != ER_SUCCESS)
        return (0);
    if (g_http_server_partial_calls < 2)
        return (0);
    if (g_http_server_partial_total != g_http_server_expected_total)
        return (0);
    return (1);
}

FT_TEST(test_http_server_short_write_sets_error, "HTTP server detects short write")
{
    ft_http_server server;

    if (server.start("127.0.0.1", 54333) != 0)
        return (0);
    g_http_server_error_stub_called = 0;
    nw_set_send_stub(&http_server_short_write_stub);
    ft_thread server_thread_object(server_worker, &server);
    SocketConfig client_configuration;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._port = 54333;
    ft_socket client_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread_object.join();
        nw_set_send_stub(NULL);
        return (0);
    }
    const char *request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    client_socket.send_all(request_string, ft_strlen(request_string), 0);
    server_thread_object.join();
    nw_set_send_stub(NULL);
    if (g_http_server_error_stub_called == 0)
        return (0);
    return (server.get_error() == SOCKET_SEND_FAILED);
}
