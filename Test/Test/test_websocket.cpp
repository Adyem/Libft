#include "../../Networking/websocket_server.hpp"
#include "../../Networking/websocket_client.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Compression/compression.hpp"
#include "../../Encryption/encryption_sha1.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <thread>

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
