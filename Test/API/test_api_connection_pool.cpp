#include "../test_internal.hpp"
#include "../../API/api.hpp"
#include "../../API/api_internal.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

static const uint16_t g_api_pool_test_port = 54540;

struct api_pool_test_server_context
{
    std::atomic<bool> ready;
    std::atomic<int> accept_count;
    std::atomic<int> handled_requests;
    std::atomic<int> result;
};

static void api_pool_test_sleep_small(void)
{
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    return ;
}

static void api_pool_test_server(api_pool_test_server_context *context)
{
    SocketConfig server_configuration;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    int response_count;

    if (!context)
        return ;
    context->result.store(0);
    context->accept_count.store(0);
    context->handled_requests.store(0);
    server_configuration._type = SocketType::SERVER;
    ft_strlcpy(server_configuration._ip, "127.0.0.1",
            sizeof(server_configuration._ip));
    server_configuration._port = g_api_pool_test_port;
    ft_socket server_socket(server_configuration);
    if (server_socket.get_fd() < 0)
    {
        context->result.store(FT_ERR_INVALID_OPERATION);
        context->ready.store(true);
        return ;
    }
    context->ready.store(true);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
    {
        context->result.store(FT_ERR_IO);
        return ;
    }
    context->accept_count.store(1);
    response_count = 0;
    while (response_count < 2)
    {
        bool connection_active;
        char buffer[1024];
        ssize_t bytes_received;
        ft_string request_storage;
        bool header_complete;
        size_t terminator_match;

        connection_active = true;
        header_complete = false;
        terminator_match = 0;
        while (connection_active && !header_complete)
        {
            bytes_received = nw_recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0)
            {
                connection_active = false;
                break;
            }
            size_t buffer_index;

            buffer_index = 0;
            while (buffer_index < static_cast<size_t>(bytes_received))
            {
                char current_char;

                current_char = buffer[buffer_index];
                request_storage.append(current_char);
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    context->result.store(ft_string::last_operation_error());
                    connection_active = false;
                    break;
                }
                if (current_char == '\r')
                {
                    if (terminator_match == 0 || terminator_match == 2)
                        terminator_match++;
                    else
                        terminator_match = 1;
                }
                else if (current_char == '\n')
                {
                    if (terminator_match == 1 || terminator_match == 3)
                        terminator_match++;
                    else
                        terminator_match = 0;
                }
                else
                    terminator_match = 0;
                if (terminator_match == 4)
                {
                    header_complete = true;
                    break;
                }
                buffer_index++;
            }
        }
        if (!connection_active || !header_complete)
            break;
        const char *response;
        size_t response_length;
        size_t total_sent;

        response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: keep-alive\r\n\r\nHello";
        response_length = ft_strlen(response);
        total_sent = 0;
        while (connection_active && total_sent < response_length)
        {
            ssize_t bytes_sent;

            bytes_sent = nw_send(client_fd, response + total_sent,
                    response_length - total_sent, 0);
            if (bytes_sent <= 0)
            {
                connection_active = false;
                break;
            }
            total_sent += static_cast<size_t>(bytes_sent);
        }
        if (!connection_active)
            break;
        response_count++;
    }
    context->handled_requests.store(response_count);
    if (client_fd >= 0)
        nw_close(client_fd);
    return ;
}

FT_TEST(test_api_connection_pool_reuses_connections,
    "api connection pool reuses idle sockets for sequential requests")
{
    api_pool_test_server_context context;

    context.ready.store(false);
    context.accept_count.store(0);
    context.handled_requests.store(0);
    context.result.store(0);
    ft_thread server_thread(api_pool_test_server, &context);

    if (!server_thread.joinable())
        return (0);
    int wait_attempts;

    wait_attempts = 0;
    while (!context.ready.load())
    {
        if (wait_attempts > 100)
        {
            server_thread.join();
            return (0);
        }
        api_pool_test_sleep_small();
        wait_attempts++;
    }
    api_debug_reset_connection_pool_counters();
    api_connection_pool_flush();
    int first_status;
    char *first_body;

    first_status = 0;
    first_body = api_request_string("127.0.0.1", g_api_pool_test_port,
            "GET", "/pool", ft_nullptr, ft_nullptr, &first_status, 2000);
    if (!first_body)
    {
        server_thread.join();
        return (0);
    }
    FT_ASSERT_EQ(200, first_status);
    FT_ASSERT(ft_strncmp(first_body, "Hello", 5) == 0);
    cma_free(first_body);
    int second_status;
    char *second_body;

    second_status = 0;
    second_body = api_request_string("127.0.0.1", g_api_pool_test_port,
            "GET", "/pool", ft_nullptr, ft_nullptr, &second_status, 2000);
    if (!second_body)
    {
        server_thread.join();
        return (0);
    }
    FT_ASSERT_EQ(200, second_status);
    FT_ASSERT(ft_strncmp(second_body, "Hello", 5) == 0);
    cma_free(second_body);
    api_connection_pool_flush();
    server_thread.join();
    if (context.result.load() != 0)
        return (0);
    FT_ASSERT_EQ(static_cast<size_t>(2),
            api_debug_get_connection_pool_acquires());
    FT_ASSERT_EQ(static_cast<size_t>(1),
            api_debug_get_connection_pool_reuses());
    FT_ASSERT_EQ(static_cast<size_t>(1),
            api_debug_get_connection_pool_misses());
    FT_ASSERT_EQ(1, context.accept_count.load());
    FT_ASSERT_EQ(2, context.handled_requests.load());
    return (1);
}

FT_TEST(test_api_connection_pool_disable_store_resets_http2_flag,
    "api connection pool disable store clears negotiated http2 state")
{
    api_connection_pool_handle handle;

    handle.negotiated_http2 = true;
    api_connection_pool_disable_store(handle);
    FT_ASSERT_EQ(false, handle.negotiated_http2);
    return (1);
}
