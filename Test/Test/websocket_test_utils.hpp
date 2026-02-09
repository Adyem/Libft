#ifndef WEBSOCKET_TEST_UTILS_HPP
#define WEBSOCKET_TEST_UTILS_HPP

#include "../../Networking/websocket_server.hpp"
#include "../../Networking/websocket_client.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/thread.hpp"
#include <unistd.h>
#include <cstring>
#include <cstddef>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct websocket_server_context
{
    ft_websocket_server *server;
    int result;
    int client_fd;
    ft_string message;
};

struct websocket_invalid_handshake_context
{
    int server_socket;
    int result;
};

static void websocket_server_worker(websocket_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->client_fd = -1;
    context->result = context->server->run_once(context->client_fd, context->message);
    return ;
}

#endif
