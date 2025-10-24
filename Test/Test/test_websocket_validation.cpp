#include "websocket_test_utils.hpp"

FT_TEST(test_websocket_server_rejects_unmasked_frame, "websocket server rejects unmasked frames")
{
    ft_websocket_server server;
    websocket_server_context context;
    int client_socket;
    ft_thread server_thread;
    unsigned short server_port;
    struct sockaddr_in server_address;
    ft_string handshake_request;
    const char *handshake_data;
    size_t handshake_length;
    size_t total_sent;
    ssize_t send_result;
    char response_buffer[512];
    ssize_t bytes_received;
    const char *terminator;
    unsigned char frame[16];
    size_t payload_length;

    if (server.start("127.0.0.1", 0) != 0)
        return (0);
    if (server.get_port(server_port) != 0)
        return (0);
    client_socket = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        return (0);
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (nw_inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1)
    {
        nw_close(client_socket);
        return (0);
    }
    if (nw_connect(client_socket, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) != 0)
    {
        nw_close(client_socket);
        return (0);
    }
    context.server = &server;
    context.result = -1;
    context.client_fd = -1;
    server_thread = ft_thread(websocket_server_worker, &context);
    if (server_thread.get_error() != ER_SUCCESS)
    {
        nw_close(client_socket);
        return (0);
    }
    handshake_request = "GET / HTTP/1.1\r\n";
    handshake_request.append("Host: 127.0.0.1\r\n");
    handshake_request.append("Upgrade: websocket\r\n");
    handshake_request.append("Connection: Upgrade\r\n");
    handshake_request.append("Sec-WebSocket-Version: 13\r\n");
    handshake_request.append("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n\r\n");
    handshake_data = handshake_request.c_str();
    handshake_length = handshake_request.size();
    total_sent = 0;
    while (total_sent < handshake_length)
    {
        send_result = nw_send(client_socket, handshake_data + total_sent, handshake_length - total_sent, 0);
        if (send_result <= 0)
        {
            nw_close(client_socket);
            server_thread.join();
            return (0);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    handshake_request.clear();
    while (1)
    {
        bytes_received = nw_recv(client_socket, response_buffer, sizeof(response_buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            nw_close(client_socket);
            server_thread.join();
            return (0);
        }
        response_buffer[bytes_received] = '\0';
        handshake_request.append(response_buffer);
        terminator = ft_strstr(handshake_request.c_str(), "\r\n\r\n");
        if (terminator != ft_nullptr)
            break ;
    }
    frame[0] = 0x81;
    payload_length = 5;
    frame[1] = static_cast<unsigned char>(payload_length);
    frame[2] = 'h';
    frame[3] = 'e';
    frame[4] = 'l';
    frame[5] = 'l';
    frame[6] = 'o';
    total_sent = 0;
    while (total_sent < payload_length + 2)
    {
        send_result = nw_send(client_socket, frame + total_sent, (payload_length + 2) - total_sent, 0);
        if (send_result <= 0)
        {
            nw_close(client_socket);
            server_thread.join();
            return (0);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    server_thread.join();
    nw_close(client_socket);
    if (context.client_fd >= 0)
        nw_close(context.client_fd);
    if (context.result == 0)
        return (0);
    if (server.get_error() != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}
