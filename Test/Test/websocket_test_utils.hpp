#ifndef WEBSOCKET_TEST_UTILS_HPP
#define WEBSOCKET_TEST_UTILS_HPP

#include "../../Modules/Networking/websocket_server.hpp"
#include "../../Modules/Networking/websocket_client.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/Networking/socket_class.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Threading/thread.hpp"
#include <unistd.h>
#include <cstring>
#include <cstddef>
#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
#endif

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
    if (context->message.is_initialised() == FT_FALSE)
    {
        if (context->message.initialize() != FT_ERR_SUCCESS)
        {
            context->result = -1;
            return ;
        }
    }
    context->result = context->server->run_once(context->client_fd, context->message);
    return ;
}

#endif
