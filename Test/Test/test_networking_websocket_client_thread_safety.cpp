#include "../test_internal.hpp"
#include "../../Modules/Networking/websocket_client.hpp"
#include "../../Modules/Networking/websocket_server.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/thread.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include <thread>

#ifndef LIBFT_TEST_BUILD
#endif

struct websocket_client_server_context
{
    ft_websocket_server *server;
    int result;
    int client_fd;
    ft_string message;
};

static void websocket_client_server_run(websocket_client_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->client_fd = -1;
    context->result = context->server->run_once(context->client_fd, context->message);
    return ;
}

FT_TEST(test_networking_websocket_client_thread_safe_error_queries)
{
    ft_websocket_server server;
    websocket_client_server_context server_context;
    ft_thread server_thread;
    ft_websocket_client client;
    unsigned short server_port;
    ft_string client_message;
    ft_string server_response;
    ft_string received_message;
    int32_t string_init_error;

    if (server.initialize() != 0)
        return (0);
    if (server.start("127.0.0.1", 0) != 0)
        return (0);
    if (server.get_port(server_port) != 0)
        return (0);
    server_context.server = &server;
    server_context.result = -1;
    server_context.client_fd = -1;
    string_init_error = server_context.message.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (0);
    server_thread = ft_thread(websocket_client_server_run, &server_context);
    if (!server_thread.joinable())
        return (0);
    if (client.initialize() != 0)
    {
        server_thread.join();
        return (0);
    }
    if (client.connect("127.0.0.1", server_port, "/") != 0)
    {
        server_thread.join();
        return (0);
    }
    string_init_error = client_message.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
    {
        server_thread.join();
        client.close();
        return (0);
    }
    string_init_error = server_response.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
    {
        server_thread.join();
        client.close();
        return (0);
    }
    string_init_error = received_message.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
    {
        server_thread.join();
        client.close();
        return (0);
    }
    client_message = "threadsafe";
    if (client.send_text(client_message) != 0)
    {
        server_thread.join();
        client.close();
        return (0);
    }
    server_thread.join();
    if (server_context.client_fd < 0)
    {
        client.close();
        return (0);
    }
    if (server_context.result != 0)
    {
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    if (!(server_context.message == client_message))
    {
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    server_response = "server-message";
    if (server.send_text(server_context.client_fd, server_response) != 0)
    {
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    if (client.receive_text(received_message) != 0)
    {
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    if (!(received_message == server_response))
    {
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    client.close();
    if (server_context.client_fd >= 0)
        nw_close(server_context.client_fd);
    return (1);
}
