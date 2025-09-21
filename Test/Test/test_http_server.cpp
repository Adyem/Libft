#include "../../Networking/http_server.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include <unistd.h>

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
