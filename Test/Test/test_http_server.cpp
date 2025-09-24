#include "../../Networking/http_server.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include "../../Errno/errno.hpp"
#include <unistd.h>
#include <cerrno>

struct http_server_context
{
    ft_http_server *server;
    int result;
};

static void http_server_run_once(http_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->result = context->server->run_once();
    return ;
}

static int collect_response(int socket_fd, ft_string &response)
{
    char buffer[256];
    ssize_t bytes_received;

    response.clear();
    while (1)
    {
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            break;
        buffer[bytes_received] = '\0';
        response.append(buffer);
        if (bytes_received < static_cast<ssize_t>(sizeof(buffer) - 1))
            break;
    }
    if (response.empty())
        return (0);
    return (1);
}

FT_TEST(test_http_server_get_response, "HTTP server handles GET requests")
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_string;
    ft_string response;

    if (server.start("127.0.0.1", 54330) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "127.0.0.1";
    client_configuration._port = 54330;
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread.join();
        return (0);
    }
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    if (client_socket.send_all(request_string, ft_strlen(request_string), 0)
        != static_cast<ssize_t>(ft_strlen(request_string)))
    {
        server_thread.join();
        return (0);
    }
    if (collect_response(client_socket.get_fd(), response) == 0)
    {
        server_thread.join();
        return (0);
    }
    server_thread.join();
    if (context.result != 0)
        return (0);
    if (server.get_error() != ER_SUCCESS)
        return (0);
    if (ft_strnstr(response.c_str(), "HTTP/1.1 200 OK", response.size()) == ft_nullptr)
        return (0);
    if (ft_strnstr(response.c_str(), "GET", response.size()) == ft_nullptr)
        return (0);
    return (1);
}

FT_TEST(test_http_server_post_body_echo, "HTTP server echoes POST body")
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_headers;
    const char *body_part_one;
    const char *body_part_two;
    ft_string response;

    if (server.start("127.0.0.1", 54331) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "127.0.0.1";
    client_configuration._port = 54331;
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread.join();
        return (0);
    }
    request_headers = "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nConnection: close\r\n\r\n";
    body_part_one = "He";
    body_part_two = "llo";
    if (client_socket.send_all(request_headers, ft_strlen(request_headers), 0)
        != static_cast<ssize_t>(ft_strlen(request_headers)))
    {
        server_thread.join();
        return (0);
    }
    usleep(50000);
    if (client_socket.send_all(body_part_one, ft_strlen(body_part_one), 0)
        != static_cast<ssize_t>(ft_strlen(body_part_one)))
    {
        server_thread.join();
        return (0);
    }
    usleep(50000);
    if (client_socket.send_all(body_part_two, ft_strlen(body_part_two), 0)
        != static_cast<ssize_t>(ft_strlen(body_part_two)))
    {
        server_thread.join();
        return (0);
    }
    if (collect_response(client_socket.get_fd(), response) == 0)
    {
        server_thread.join();
        return (0);
    }
    server_thread.join();
    if (context.result != 0)
        return (0);
    if (server.get_error() != ER_SUCCESS)
        return (0);
    if (ft_strnstr(response.c_str(), "Hello", response.size()) == ft_nullptr)
        return (0);
    return (1);
}

FT_TEST(test_http_server_short_write_sets_error, "HTTP server detects closed clients")
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_string;
    int error_code;

    if (server.start("127.0.0.1", 54332) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "127.0.0.1";
    client_configuration._port = 54332;
    client_socket = ft_socket(client_configuration);
    if (client_socket.get_error() != ER_SUCCESS)
    {
        server_thread.join();
        return (0);
    }
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    client_socket.send_all(request_string, ft_strlen(request_string), 0);
    client_socket.close_socket();
    server_thread.join();
    if (context.result == 0)
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
