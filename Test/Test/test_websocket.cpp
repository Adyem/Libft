#include "../../Networking/websocket_server.hpp"
#include "../../Networking/websocket_client.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/thread.hpp"
#include <unistd.h>
#include <cstring>

struct websocket_server_context
{
    ft_websocket_server *server;
    int result;
    int client_fd;
    ft_string message;
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

struct websocket_invalid_server_context
{
    uint16_t port;
    int status;
};

static void websocket_invalid_handshake_server(websocket_invalid_server_context *context)
{
    int listen_socket;
    struct sockaddr_in server_address;
    int reuse_value;
    int option_result;
    struct sockaddr_storage client_address;
    socklen_t client_length;
    int client_socket;
    char buffer[512];
    ssize_t bytes_received;
    ft_string request;
    const char *terminator;
    const char *response;
    size_t total_sent;
    size_t response_length;
    ssize_t send_result;

    if (context == ft_nullptr)
        return ;
    context->status = 0;
    listen_socket = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        return ;
    reuse_value = 1;
#ifdef _WIN32
    option_result = setsockopt(static_cast<SOCKET>(listen_socket), SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<const char *>(&reuse_value), sizeof(reuse_value));
    if (option_result == SOCKET_ERROR)
#else
    option_result = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_value, sizeof(reuse_value));
    if (option_result != 0)
#endif
    {
        FT_CLOSE_SOCKET(listen_socket);
        return ;
    }
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(context->port);
    if (nw_inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1)
    {
        FT_CLOSE_SOCKET(listen_socket);
        return ;
    }
    if (nw_bind(listen_socket, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) != 0)
    {
        FT_CLOSE_SOCKET(listen_socket);
        return ;
    }
    if (nw_listen(listen_socket, 1) != 0)
    {
        FT_CLOSE_SOCKET(listen_socket);
        return ;
    }
    client_length = sizeof(client_address);
    client_socket = nw_accept(listen_socket, reinterpret_cast<struct sockaddr*>(&client_address), &client_length);
    FT_CLOSE_SOCKET(listen_socket);
    if (client_socket < 0)
        return ;
    request.clear();
    while (1)
    {
        bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            return ;
        }
        buffer[bytes_received] = '\0';
        request.append(buffer);
        terminator = ft_strstr(request.c_str(), "\r\n\r\n");
        if (terminator != ft_nullptr)
            break;
    }
    response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    response_length = ft_strlen(response);
    total_sent = 0;
    while (total_sent < response_length)
    {
        send_result = nw_send(client_socket, response + total_sent, response_length - total_sent, 0);
        if (send_result <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            return ;
        }
        total_sent += static_cast<size_t>(send_result);
    }
    FT_CLOSE_SOCKET(client_socket);
    context->status = 1;
    return ;
}

FT_TEST(test_websocket_handshake_and_echo, "websocket server echoes message")
{
    ft_websocket_server server;
    websocket_server_context context;
    ft_thread server_thread;
    ft_websocket_client client;
    ft_string reply;
    ft_string received;

    if (server.start("127.0.0.1", 54350) != 0)
        return (0);
    context.server = &server;
    context.result = -1;
    context.client_fd = -1;
    server_thread = ft_thread(websocket_server_worker, &context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    if (client.connect("127.0.0.1", 54350, "/") != 0)
    {
        server_thread.join();
        return (0);
    }
    ft_string message;
    message = "ping";
    if (client.send_text(message) != 0)
    {
        server_thread.join();
        client.close();
        return (0);
    }
    server_thread.join();
    if (context.result != 0)
    {
        client.close();
        if (context.client_fd >= 0)
            FT_CLOSE_SOCKET(context.client_fd);
        return (0);
    }
    if (!(context.message == message))
    {
        client.close();
        if (context.client_fd >= 0)
            FT_CLOSE_SOCKET(context.client_fd);
        return (0);
    }
    reply = "pong";
    if (server.send_text(context.client_fd, reply) != 0)
    {
        client.close();
        if (context.client_fd >= 0)
            FT_CLOSE_SOCKET(context.client_fd);
        return (0);
    }
    if (client.receive_text(received) != 0)
    {
        client.close();
        if (context.client_fd >= 0)
            FT_CLOSE_SOCKET(context.client_fd);
        return (0);
    }
    client.close();
    if (context.client_fd >= 0)
        FT_CLOSE_SOCKET(context.client_fd);
    return (received == reply);
}

FT_TEST(test_websocket_server_handles_fragmented_handshake, "websocket server handles fragmented handshake")
{
    ft_websocket_server server;
    websocket_server_context context;
    int client_socket;
    struct sockaddr_in server_address;
    ft_thread server_thread;
    ft_string handshake_request;
    const char *handshake_data;
    size_t handshake_length;
    size_t total_sent;
    size_t send_length;
    ssize_t send_result;
    char response_buffer[512];
    ft_string handshake_response;
    ssize_t bytes_received;
    const char *terminator;
    ft_string expected_message;
    unsigned char frame[64];
    size_t payload_length;
    size_t index_value;
    size_t mask_index;
    size_t frame_length;

    if (server.start("127.0.0.1", 54355) != 0)
        return (0);
    client_socket = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        return (0);
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(54355);
    if (nw_inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1)
    {
        FT_CLOSE_SOCKET(client_socket);
        return (0);
    }
    if (nw_connect(client_socket, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) != 0)
    {
        FT_CLOSE_SOCKET(client_socket);
        return (0);
    }
    context.server = &server;
    context.result = -1;
    context.client_fd = -1;
    server_thread = ft_thread(websocket_server_worker, &context);
    if (server_thread.get_error() != ER_SUCCESS)
    {
        FT_CLOSE_SOCKET(client_socket);
        return (0);
    }
    handshake_request = "GET / HTTP/1.1\r\n";
    handshake_request.append("Host: 127.0.0.1\r\n");
    handshake_request.append("Upgrade: websocket\r\n");
    handshake_request.append("Connection: Upgrade\r\n");
    handshake_request.append("Sec-WebSocket-Version: 13\r\n");
    handshake_request.append("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n");
    handshake_data = handshake_request.c_str();
    handshake_length = handshake_request.size();
    total_sent = 0;
    while (total_sent < handshake_length)
    {
        if (total_sent == 0)
            send_length = 8;
        else if (total_sent < 20)
            send_length = 7;
        else
            send_length = handshake_length - total_sent;
        if (send_length > handshake_length - total_sent)
            send_length = handshake_length - total_sent;
        send_result = nw_send(client_socket, handshake_data + total_sent, send_length, 0);
        if (send_result <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            server_thread.join();
            return (0);
        }
        total_sent += static_cast<size_t>(send_result);
        if (total_sent < handshake_length)
            usleep(50000);
    }
    handshake_response.clear();
    while (1)
    {
        bytes_received = nw_recv(client_socket, response_buffer, sizeof(response_buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            server_thread.join();
            return (0);
        }
        response_buffer[bytes_received] = '\0';
        handshake_response.append(response_buffer);
        terminator = ft_strstr(handshake_response.c_str(), "\r\n\r\n");
        if (terminator != ft_nullptr)
            break;
    }
    expected_message = "split";
    payload_length = expected_message.size();
    frame[0] = 0x81;
    frame[1] = static_cast<unsigned char>(0x80 | payload_length);
    frame[2] = 0x12;
    frame[3] = 0x34;
    frame[4] = 0x56;
    frame[5] = 0x78;
    index_value = 0;
    while (index_value < payload_length)
    {
        mask_index = index_value % 4;
        frame[6 + index_value] = static_cast<unsigned char>(expected_message.c_str()[index_value] ^ frame[2 + mask_index]);
        index_value++;
    }
    frame_length = 6 + payload_length;
    total_sent = 0;
    while (total_sent < frame_length)
    {
        send_result = nw_send(client_socket, frame + total_sent, frame_length - total_sent, 0);
        if (send_result <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            server_thread.join();
            return (0);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    server_thread.join();
    FT_CLOSE_SOCKET(client_socket);
    if (context.client_fd >= 0)
        FT_CLOSE_SOCKET(context.client_fd);
    return (context.result == 0 && context.message == expected_message);
}

FT_TEST(test_websocket_client_rejects_invalid_handshake, "websocket client detects invalid handshake")
{
    websocket_invalid_server_context server_context;
    ft_thread server_thread;
    ft_websocket_client client;
    int connect_result;

    server_context.port = 54351;
    server_context.status = 0;
    server_thread = ft_thread(websocket_invalid_handshake_server, &server_context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    usleep(50000);
    connect_result = client.connect("127.0.0.1", 54351, "/test");
    server_thread.join();
    client.close();
    if (server_context.status == 0)
        return (0);
    if (connect_result == 0)
        return (0);
    if (client.get_error() != FT_EINVAL)
        return (0);
    return (1);
}
