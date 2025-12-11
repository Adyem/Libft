#include "../../Networking/http_client.hpp"
#include "../../Networking/http_server.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/thread.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"

struct http_client_server_context
{
    ft_http_server *server;
    int result;
};

static void http_client_server_run_once(http_client_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->result = context->server->run_once();
    return ;
}

FT_TEST(test_http_client_reuses_keep_alive_connection, "HTTP client reuses pooled keep-alive sockets")
{
    ft_http_server server;
    http_client_server_context context;
    ft_thread server_thread;
    ft_string response;
    int first_result;
    int second_result;

    http_client_pool_flush();
    http_client_pool_debug_reset_counters();
    if (server.start("127.0.0.1", 54340) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_client_server_run_once, &context);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
    {
        return (0);
    }
    first_result = http_get("127.0.0.1", "/first", response, false, "54340");
    if (first_result != 0)
    {
        http_client_pool_flush();
        server_thread.join();
        return (0);
    }
    if (http_client_pool_get_idle_count() != 1)
    {
        http_client_pool_flush();
        server_thread.join();
        return (0);
    }
    if (http_client_pool_debug_get_miss_count() == 0)
    {
        http_client_pool_flush();
        server_thread.join();
        return (0);
    }
    response.clear();
    second_result = http_get("127.0.0.1", "/second", response, false, "54340");
    if (second_result != 0)
    {
        http_client_pool_flush();
        server_thread.join();
        return (0);
    }
    if (http_client_pool_debug_get_reuse_count() == 0)
    {
        http_client_pool_flush();
        server_thread.join();
        return (0);
    }
    http_client_pool_flush();
    server_thread.join();
    if (context.result != 0 && server.get_error() != FT_ERR_SOCKET_SEND_FAILED)
        return (0);
    if (http_client_pool_get_idle_count() != 0)
        return (0);
    return (1);
}
