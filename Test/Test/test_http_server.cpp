#include "../../Networking/http_server.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include "../../Errno/errno.hpp"
#include "network_io_harness.hpp"
#include <unistd.h>
#include <thread>
#include <chrono>

struct http_client_read_result
{
    ft_string data;
    size_t read_iterations;
    int status;
};

static void slow_client_read(int descriptor, size_t expected_length, int delay_milliseconds,
                             http_client_read_result *result)
{
    char buffer[32];

    if (result == ft_nullptr)
        return ;
    result->data.clear();
    result->read_iterations = 0;
    result->status = 0;
    while (result->data.size() < expected_length)
    {
        size_t remaining_length;
        size_t bytes_to_read;
        ssize_t read_result;

        remaining_length = expected_length - result->data.size();
        if (remaining_length < sizeof(buffer))
            bytes_to_read = remaining_length;
        else
            bytes_to_read = sizeof(buffer);
        read_result = nw_recv(descriptor, buffer, bytes_to_read, 0);
        if (read_result <= 0)
        {
            if (read_result < 0)
                result->status = -1;
            return ;
        }
        size_t buffer_index;

        buffer_index = 0;
        while (buffer_index < static_cast<size_t>(read_result))
        {
            result->data.append(buffer[buffer_index]);
            buffer_index++;
        }
        result->read_iterations++;
        if (delay_milliseconds > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_milliseconds));
    }
    return ;
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
    network_io_harness harness;
    ft_thread server_thread_object;
    size_t expected_length;
    http_client_read_result read_result;
    std::thread reader_thread;
    const char *request_string;
    ssize_t send_result;

    if (server.start("127.0.0.1", 54332) != 0)
        return (0);
    expected_response = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nGET";
    server_thread_object = ft_thread(server_worker, &server);
    if (harness.connect_remote("127.0.0.1", 54332) != ER_SUCCESS)
    {
        server_thread_object.join();
        return (0);
    }
    if (harness.set_client_receive_buffer(32) != ER_SUCCESS)
    {
        server_thread_object.join();
        return (0);
    }
    expected_length = ft_strlen(expected_response);
    reader_thread = std::thread([&harness, expected_length, &read_result]()
    {
        slow_client_read(harness.get_client_fd(), expected_length, 10, &read_result);
    });
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send_result = harness.get_client_socket().send_all(request_string, ft_strlen(request_string), 0);
    if (send_result < 0)
    {
        if (reader_thread.joinable())
            reader_thread.join();
        server_thread_object.join();
        return (0);
    }
    server_thread_object.join();
    if (reader_thread.joinable())
        reader_thread.join();
    if (server.get_error() != ER_SUCCESS)
        return (0);
    if (read_result.status != 0)
        return (0);
    if (!(read_result.data == expected_response))
        return (0);
    return (read_result.read_iterations > 1);
}

FT_TEST(test_http_server_short_write_sets_error, "HTTP server detects short write")
{
    ft_http_server server;
    network_io_harness harness;
    ft_thread server_thread_object;
    const char *request_string;
    std::thread closer_thread;
    ssize_t send_result;
    int error_code;

    if (server.start("127.0.0.1", 54333) != 0)
        return (0);
    server_thread_object = ft_thread(server_worker, &server);
    if (harness.connect_remote("127.0.0.1", 54333) != ER_SUCCESS)
    {
        server_thread_object.join();
        return (0);
    }
    closer_thread = std::thread([&harness]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        harness.close_client();
    });
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send_result = harness.get_client_socket().send_all(request_string, ft_strlen(request_string), 0);
    if (closer_thread.joinable())
        closer_thread.join();
    server_thread_object.join();
    if (send_result < 0)
        return (0);
    error_code = server.get_error();
    if (error_code == SOCKET_SEND_FAILED)
        return (1);
#ifdef _WIN32
    if (error_code == (WSAECONNRESET + ERRNO_OFFSET)
        || error_code == (WSAENOTCONN + ERRNO_OFFSET)
        || error_code == (WSAECONNABORTED + ERRNO_OFFSET))
        return (1);
#else
    if (error_code == (ECONNRESET + ERRNO_OFFSET))
        return (1);
#ifdef EPIPE
    if (error_code == (EPIPE + ERRNO_OFFSET))
        return (1);
#endif
#endif
    return (0);
}
