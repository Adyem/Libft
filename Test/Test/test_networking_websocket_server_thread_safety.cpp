#include "../test_internal.hpp"
#include "../../Networking/websocket_server.hpp"
#include "../../Networking/websocket_client.hpp"
#include "../../Networking/networking.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/thread.hpp"
#include <thread>
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

struct websocket_server_thread_context
{
    ft_websocket_server *server;
    int result;
    int client_fd;
    ft_string message;
};

struct websocket_server_inspector_context
{
    ft_websocket_server *server;
    std::atomic<bool> *running;
    std::atomic<bool> *failed;
};

static void websocket_server_run_once(websocket_server_thread_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->client_fd = -1;
    context->result = context->server->run_once(context->client_fd, context->message);
    return ;
}

static void websocket_server_inspector(websocket_server_inspector_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    if (context->running == ft_nullptr)
        return ;
    if (context->failed == ft_nullptr)
        return ;
    while (context->running->load())
    {
        int error_code;
        const char *error_string;

        error_code = context->server->get_error();
        if (error_code < 0)
        {
            context->failed->store(true);
            return ;
        }
        error_string = context->server->get_error_str();
        if (error_string == ft_nullptr)
        {
            context->failed->store(true);
            return ;
        }
    }
    return ;
}

FT_TEST(test_websocket_server_thread_safe_error_queries,
    "ft_websocket_server exposes errors safely under concurrent inspection")
{
    ft_websocket_server server;
    websocket_server_thread_context context;
    ft_thread server_thread;
    ft_websocket_client client;
    unsigned short server_port;
    ft_string message;
    std::atomic<bool> inspector_running;
    std::atomic<bool> inspector_failed;
    websocket_server_inspector_context inspector_context;
    std::thread inspector_thread;

    if (server.start("127.0.0.1", 0) != 0)
        return (0);
    if (server.get_port(server_port) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    context.client_fd = -1;
    server_thread = ft_thread(websocket_server_run_once, &context);
    if (server_thread.get_error() != FT_ERR_SUCCESS)
        return (0);
    inspector_running.store(true);
    inspector_failed.store(false);
    inspector_context.server = &server;
    inspector_context.running = &inspector_running;
    inspector_context.failed = &inspector_failed;
    inspector_thread = std::thread(websocket_server_inspector, &inspector_context);
    if (client.connect("127.0.0.1", server_port, "/") != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        server_thread.join();
        return (0);
    }
    message = "threadsafe";
    if (client.send_text(message) != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        server_thread.join();
        client.close();
        return (0);
    }
    server_thread.join();
    inspector_running.store(false);
    inspector_thread.join();
    if (inspector_failed.load())
    {
        client.close();
        if (context.client_fd >= 0)
            nw_close(context.client_fd);
        return (0);
    }
    if (context.result != 0)
    {
        client.close();
        if (context.client_fd >= 0)
            nw_close(context.client_fd);
        return (0);
    }
    if (!(context.message == message))
    {
        client.close();
        if (context.client_fd >= 0)
            nw_close(context.client_fd);
        return (0);
    }
    client.close();
    if (context.client_fd >= 0)
        nw_close(context.client_fd);
    return (1);
}
