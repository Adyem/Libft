#include "../../Networking/websocket_server.hpp"
#include "../../Networking/websocket_client.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Compression/compression.hpp"
#include "../../Encryption/encryption_sha1.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <thread>
#include <chrono>
#include <cstring>

static ssize_t websocket_handshake_short_write_stub(int socket_fd, const void *buffer,
                                                    size_t length, int flags)
{
    const char *char_buffer;

    (void)socket_fd;
    (void)flags;
    char_buffer = static_cast<const char *>(buffer);
    if (length >= 3 && ft_strncmp(char_buffer, "GET", 3) == 0)
        return (0);
    return (static_cast<ssize_t>(length));
}

static void websocket_split_handshake_server(uint16_t port, int *result)
{
    int listen_socket;
    struct sockaddr_in server_address;
    struct sockaddr_storage client_address;
    socklen_t client_length;
    int client_socket;
    char buffer[1024];
    ssize_t bytes_received;
    ft_string request;
    const char *header_terminator;
    const char *key_line;
    const char *key_end;
    ft_string key_value;
    ft_string magic;
    unsigned char digest[20];
    unsigned char *encoded_accept;
    std::size_t encoded_size;
    ft_string accept_value;
    ft_string response;
    const char *response_data;
    size_t first_chunk;
    size_t bytes_to_send;
    size_t sent_total;
    ssize_t send_result;
    int reuse_value;
    int option_result;

    *result = 0;
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
    server_address.sin_port = htons(port);
    if (nw_inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1)
    {
        FT_CLOSE_SOCKET(listen_socket);
        return ;
    }
    if (nw_bind(listen_socket, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) != 0)
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
    client_socket = nw_accept(listen_socket, reinterpret_cast<struct sockaddr *>(&client_address), &client_length);
    FT_CLOSE_SOCKET(listen_socket);
    if (client_socket < 0)
        return ;
    request.clear();
    while (true)
    {
        bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            return ;
        }
        buffer[bytes_received] = '\0';
        request.append(buffer);
        header_terminator = ft_strstr(request.c_str(), "\r\n\r\n");
        if (header_terminator)
            break;
    }
    key_line = ft_strstr(request.c_str(), "Sec-WebSocket-Key: ");
    if (!key_line)
    {
        FT_CLOSE_SOCKET(client_socket);
        return ;
    }
    key_line += ft_strlen("Sec-WebSocket-Key: ");
    key_end = ft_strstr(key_line, "\r\n");
    key_value.clear();
    if (key_end)
    {
        size_t key_index;

        key_index = 0;
        while (key_line + key_index < key_end)
        {
            key_value.append(key_line[key_index]);
            key_index++;
        }
    }
    else
    {
        size_t key_index;

        key_index = 0;
        while (key_line[key_index] != '\0')
        {
            key_value.append(key_line[key_index]);
            key_index++;
        }
    }
    magic = key_value;
    magic.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    sha1_hash(magic.c_str(), magic.size(), digest);
    encoded_accept = ft_base64_encode(digest, 20, &encoded_size);
    if (!encoded_accept)
    {
        FT_CLOSE_SOCKET(client_socket);
        return ;
    }
    accept_value.clear();
    size_t accept_index;

    accept_index = 0;
    while (accept_index < encoded_size)
    {
        accept_value.append(reinterpret_cast<char *>(encoded_accept)[accept_index]);
        accept_index++;
    }
    cma_free(encoded_accept);
    response.clear();
    response.append("HTTP/1.1 101 Switching Protocols\r\n");
    response.append("Upgrade: websocket\r\n");
    response.append("Connection: Upgrade\r\n");
    response.append("Sec-WebSocket-Accept: ");
    response.append(accept_value);
    response.append("\r\n\r\n");
    response_data = response.c_str();
    first_chunk = 5;
    if (first_chunk >= response.size())
    {
        first_chunk = response.size() / 2;
        if (first_chunk == 0 && response.size() > 0)
            first_chunk = response.size();
    }
    sent_total = 0;
    bytes_to_send = first_chunk;
    while (sent_total < bytes_to_send)
    {
        send_result = nw_send(client_socket, response_data + sent_total, bytes_to_send - sent_total, 0);
        if (send_result <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            return ;
        }
        sent_total += static_cast<size_t>(send_result);
    }
    bytes_to_send = response.size() - first_chunk;
    sent_total = 0;
    while (sent_total < bytes_to_send)
    {
        send_result = nw_send(client_socket, response_data + first_chunk + sent_total, bytes_to_send - sent_total, 0);
        if (send_result <= 0)
        {
            FT_CLOSE_SOCKET(client_socket);
            return ;
        }
        sent_total += static_cast<size_t>(send_result);
    }
    FT_CLOSE_SOCKET(client_socket);
    *result = 1;
    return ;
}

static void websocket_client_worker(uint16_t port, ft_string *message, ft_string *key, int *result)
{
    ft_websocket_client client;

    client.set_handshake_key_override(*key);
    if (client.connect("127.0.0.1", port, "/") != 0)
    {
        client.close();
        *result = 1;
        return ;
    }
    if (client.send_text(*message) != 0)
    {
        client.close();
        *result = 1;
        return ;
    }
    client.close();
    *result = 0;
    return ;
}

FT_TEST(test_websocket_sha1_handshake, "websocket handshake computes SHA-1 accept key")
{
    ft_websocket_server server;
    ft_string received_message;
    ft_string known_key;
    ft_string message_to_send;
    int client_result;
    int server_result;
    uint16_t port;

    port = 54873;
    if (server.start("127.0.0.1", port) != 0)
        return (0);
    known_key = "dGhlIHNhbXBsZSBub25jZQ==";
    message_to_send = "hello";
    client_result = -1;
    std::thread client_thread(websocket_client_worker, port, &message_to_send, &known_key, &client_result);
    int client_fd;

    client_fd = -1;
    server_result = server.run_once(client_fd, received_message);
    if (client_fd >= 0)
    {
        FT_CLOSE_SOCKET(client_fd);
    }
    if (client_thread.joinable())
    {
        client_thread.join();
    }
    if (client_result != 0 || server_result != 0)
        return (0);
    if (!(received_message == message_to_send))
        return (0);
    ft_string magic;
    magic = known_key;
    magic.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    unsigned char digest[20];
    sha1_hash(magic.c_str(), magic.size(), digest);
    std::size_t encoded_size;
    unsigned char *encoded;
    encoded = ft_base64_encode(digest, 20, &encoded_size);
    if (!encoded)
        return (0);
    ft_string expected_accept;
    expected_accept.clear();
    std::size_t index_value;
    index_value = 0;
    while (index_value < encoded_size)
    {
        expected_accept.append(reinterpret_cast<char *>(encoded)[index_value]);
        index_value++;
    }
    cma_free(encoded);
    ft_string known_accept;

    known_accept = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
    if (!(expected_accept == known_accept))
        return (0);
    return (1);
}

FT_TEST(test_websocket_handshake_split_response, "websocket handshake handles split response")
{
    uint16_t port;
    int server_result;
    ft_websocket_client client;
    int connect_result;

    port = 54875;
    server_result = 0;
    std::thread server_thread(websocket_split_handshake_server, port, &server_result);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    connect_result = client.connect("127.0.0.1", port, "/");
    client.close();
    if (server_thread.joinable())
    {
        server_thread.join();
    }
    if (server_result != 1)
        return (0);
    return (connect_result == 0);
}

FT_TEST(test_websocket_handshake_short_write_sets_error, "websocket handshake detects short write")
{
    ft_websocket_server server;
    ft_websocket_client client;
    uint16_t port;
    int connect_result;
    int error_code;

    port = 54874;
    if (server.start("127.0.0.1", port) != 0)
        return (0);
    nw_set_send_stub(&websocket_handshake_short_write_stub);
    connect_result = client.connect("127.0.0.1", port, "/");
    error_code = client.get_error();
    nw_set_send_stub(NULL);
    client.close();
    if (connect_result == 0)
        return (0);
    return (error_code == SOCKET_SEND_FAILED);
}
