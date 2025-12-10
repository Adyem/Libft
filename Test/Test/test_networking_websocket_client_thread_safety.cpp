#include "../../Networking/websocket_client.hpp"
#include "../../Networking/websocket_server.hpp"
#include "../../Networking/networking.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/thread.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <thread>
#include <atomic>

struct websocket_client_server_context
{
    ft_websocket_server *server;
    int result;
    int client_fd;
    ft_string message;
};

struct websocket_client_inspector_context
{
    ft_websocket_client *client;
    std::atomic<bool> *running;
    std::atomic<bool> *failed;
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

static void websocket_client_inspector(websocket_client_inspector_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->client == ft_nullptr)
        return ;
    if (context->running == ft_nullptr)
        return ;
    if (context->failed == ft_nullptr)
        return ;
    while (context->running->load())
    {
        int error_code;
        const char *error_string;

        error_code = context->client->get_error();
        if (error_code < 0)
        {
            context->failed->store(true);
            return ;
        }
        error_string = context->client->get_error_str();
        if (error_string == ft_nullptr)
        {
            context->failed->store(true);
            return ;
        }
    }
    return ;
}

FT_TEST(test_websocket_client_thread_safe_error_queries,
    "ft_websocket_client exposes errors safely while performing websocket operations")
{
    ft_websocket_server server;
    websocket_client_server_context server_context;
    ft_thread server_thread;
    ft_websocket_client client;
    unsigned short server_port;
    std::atomic<bool> inspector_running;
    std::atomic<bool> inspector_failed;
    websocket_client_inspector_context inspector_context;
    std::thread inspector_thread;
    ft_string client_message;
    ft_string server_response;
    ft_string received_message;

    if (server.start("127.0.0.1", 0) != 0)
        return (0);
    if (server.get_port(server_port) != 0)
        return (0);
    server_context.server = &server;
    server_context.result = -1;
    server_context.client_fd = -1;
    server_thread = ft_thread(websocket_client_server_run, &server_context);
    if (server_thread.get_error() != FT_ER_SUCCESSS)
        return (0);
    inspector_running.store(true);
    inspector_failed.store(false);
    inspector_context.client = &client;
    inspector_context.running = &inspector_running;
    inspector_context.failed = &inspector_failed;
    inspector_thread = std::thread(websocket_client_inspector, &inspector_context);
    if (client.connect("127.0.0.1", server_port, "/") != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        server_thread.join();
        return (0);
    }
    client_message = "threadsafe";
    if (client.send_text(client_message) != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        server_thread.join();
        client.close();
        return (0);
    }
    server_thread.join();
    if (server_context.client_fd < 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        client.close();
        return (0);
    }
    if (server_context.result != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    if (!(server_context.message == client_message))
    {
        inspector_running.store(false);
        inspector_thread.join();
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    server_response = "server-message";
    if (server.send_text(server_context.client_fd, server_response) != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    if (client.receive_text(received_message) != 0)
    {
        inspector_running.store(false);
        inspector_thread.join();
        client.close();
        if (server_context.client_fd >= 0)
            nw_close(server_context.client_fd);
        return (0);
    }
    inspector_running.store(false);
    inspector_thread.join();
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
