#include "../../API/api.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../PThread/thread.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <csignal>
#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

static void api_request_noop_callback(char *body, int status, void *user_data)
{
    (void)body;
    (void)status;
    (void)user_data;
    return ;
}

static void api_request_send_failure_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54337;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd >= 0)
        FT_CLOSE_SOCKET(client_fd);
    return ;
}

static void api_request_small_delay(void)
{
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    return ;
}

static void api_request_success_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *response;
    size_t response_length;
    size_t total_sent;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54338;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
                          reinterpret_cast<struct sockaddr*>(&address_storage),
                          &address_length);
    if (client_fd < 0)
        return ;
    response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
    response_length = ft_strlen(response);
    total_sent = 0;
    while (total_sent < response_length)
    {
        ssize_t bytes_sent;

        bytes_sent = nw_send(client_fd, response + total_sent,
                             response_length - total_sent, 0);
        if (bytes_sent <= 0)
            break;
        total_sent += static_cast<size_t>(bytes_sent);
    }
    FT_CLOSE_SOCKET(client_fd);
    return ;
}

FT_TEST(test_api_request_invalid_ip_sets_socket_error, "api_request_string invalid ip sets socket error")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string("bad-ip", 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 10);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != SOCKET_INVALID_CONFIGURATION)
        return (0);
    return (1);
}

FT_TEST(test_api_request_connect_failure_sets_errno, "api_request_string_host connect failure sets errno")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string_host("127.0.0.1", 59999, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 50);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != SOCKET_CONNECT_FAILED)
        return (0);
    return (1);
}

FT_TEST(test_api_request_send_failure_sets_errno, "api_request_string send failure sets errno")
{
    char *result;
    ft_thread server_thread;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = ER_SUCCESS;
    server_thread = ft_thread(api_request_send_failure_server);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    result = api_request_string("127.0.0.1", 54337, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    int request_errno = ft_errno;
    server_thread.join();
    if (result != ft_nullptr)
        return (0);
    if (request_errno != SOCKET_SEND_FAILED && request_errno != (EPIPE + ERRNO_OFFSET))
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    ft_errno = request_errno;
    return (1);
}

FT_TEST(test_api_request_async_alloc_failure_sets_errno, "api_request_string_async allocation failure sets errno")
{
    bool result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    result = api_request_string_async("127.0.0.1", 8080, "GET", "/", api_request_noop_callback,
                                      ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    cma_set_alloc_limit(0);
    if (result)
        return (0);
    if (ft_errno != FT_EALLOC)
        return (0);
    return (1);
}

FT_TEST(test_api_request_bad_input_sets_errno, "api_request_string_host bad input sets errno")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string_host(ft_nullptr, 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 10);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_api_request_success_resets_errno, "api_request_string success resets errno")
{
    char *body;
    ft_thread server_thread;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = FT_EALLOC;
    server_thread = ft_thread(api_request_success_server);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    body = api_request_string("127.0.0.1", 54338, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    server_thread.join();
    if (body == ft_nullptr)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    cma_free(body);
    return (1);
}

FT_TEST(test_api_request_string_url_invalid_sets_errno, "api_request_string_url invalid input sets errno")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string_url("example.com/path", "GET", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_api_request_tls_missing_host_sets_errno, "api_request_string_tls missing host sets errno")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string_tls(ft_nullptr, 443, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_api_request_async_missing_callback_sets_errno, "api_request_string_async missing callback sets errno")
{
    bool result;

    ft_errno = ER_SUCCESS;
    result = api_request_string_async("127.0.0.1", 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_api_request_async_success_resets_errno, "api_request_string_async success resets errno")
{
    bool result;

    ft_errno = FT_EALLOC;
    result = api_request_string_async("127.0.0.1", 59999, "GET", "/", api_request_noop_callback,
                                      ft_nullptr, ft_nullptr, ft_nullptr, 100);
    if (!result)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    return (1);
}

