#include "websocket_client.hpp"
#include "networking.hpp"
#include "socket_class.hpp"
#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../RNG/rng.hpp"
#include "../Encryption/basic_encryption.hpp"
#include "../Encryption/encryption_sha1.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <unistd.h>
#endif

static void compute_accept_key(const ft_string &key, ft_string &accept)
{
    unsigned char digest[20];
    unsigned char *encoded;
    std::size_t encoded_size;
    ft_string magic;

    magic = key;
    magic.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    sha1_hash(magic.c_str(), magic.size(), digest);
    accept.clear();
    encoded = ft_base64_encode(digest, 20, &encoded_size);
    if (encoded)
    {
        std::size_t index_value = 0;
        while (index_value < encoded_size)
        {
            accept.append(reinterpret_cast<char *>(encoded)[index_value]);
            index_value++;
        }
        cma_free(encoded);
    }
    return ;
}

ft_websocket_client::ft_websocket_client() : _socket(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_websocket_client::~ft_websocket_client()
{
    this->close();
    return ;
}

void ft_websocket_client::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void ft_websocket_client::close()
{
    if (!this->_socket.close())
    {
        this->set_error(this->_socket.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_websocket_client::perform_handshake(const char *host, const char *path)
{
    std::size_t byte_index;
    std::size_t shift_index;
    ft_string key_string;
    ft_string request;
    char buffer[1024];
    ssize_t bytes_received;
    ft_string response;
    const char *accept_line;
    const char *line_end;
    ft_string accept_key;
    ft_string expected;

    unsigned char random_key[16];
    std::size_t encoded_size;
    unsigned char *encoded_key;
    int socket_fd;

    socket_fd = this->_socket.get();
    if (socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }

    byte_index = 0;
    while (byte_index < 16)
    {
        uint32_t random_value = ft_random_uint32();
        shift_index = 0;
        while (shift_index < 4 && byte_index < 16)
        {
            random_key[byte_index] = static_cast<unsigned char>(random_value >> (shift_index * 8));
            byte_index++;
            shift_index++;
        }
    }
    encoded_key = ft_base64_encode(random_key, 16, &encoded_size);
    if (!encoded_key)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    key_string.clear();
    byte_index = 0;
    while (byte_index < encoded_size)
    {
        key_string.append(reinterpret_cast<char *>(encoded_key)[byte_index]);
        byte_index++;
    }
    cma_free(encoded_key);
    request.append("GET ");
    request.append(path);
    request.append(" HTTP/1.1\r\nHost: ");
    request.append(host);
    request.append("\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: ");
    request.append(key_string);
    request.append("\r\n\r\n");
    const char *request_data;
    size_t total_sent;
    ssize_t send_result;

    request_data = request.c_str();
    total_sent = 0;
    while (total_sent < request.size())
    {
        send_result = nw_send(socket_fd, request_data + total_sent, request.size() - total_sent, 0);
        if (send_result <= 0)
        {
            if (send_result < 0)
                this->set_error(ft_errno);
            else
                this->set_error(FT_ERR_SOCKET_SEND_FAILED);
            return (1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_socket_after_send(socket_fd) != 0)
    {
        this->set_error(ft_errno);
        return (1);
    }
    response.clear();
    while (true)
    {
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0)
        {
            this->set_error(ft_errno);
            return (1);
        }
        if (bytes_received == 0)
        {
            this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
            return (1);
        }
        buffer[bytes_received] = '\0';
        response.append(buffer);
        if (ft_strstr(response.c_str(), "\r\n\r\n"))
        {
            break;
        }
    }
    accept_line = ft_strstr(response.c_str(), "Sec-WebSocket-Accept: ");
    if (!accept_line)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    accept_line += ft_strlen("Sec-WebSocket-Accept: ");
    line_end = ft_strstr(accept_line, "\r\n");
    accept_key.clear();
    if (line_end)
    {
        byte_index = 0;
        while (accept_line + byte_index < line_end)
        {
            accept_key.append(accept_line[byte_index]);
            byte_index++;
        }
    }
    else
    {
        std::size_t index_value = 0;
        while (accept_line[index_value] != '\0')
        {
            accept_key.append(accept_line[index_value]);
            index_value++;
        }
    }
    compute_accept_key(key_string, expected);
    if (!(accept_key == expected))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_websocket_client::connect(const char *host, uint16_t port, const char *path)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    char port_string[8];
    int result;

    ft_memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    std::snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    int new_socket_fd;
    int connect_error;

    new_socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (new_socket_fd < 0)
    {
        freeaddrinfo(address_info);
        this->set_error(ft_errno);
        return (1);
    }
    if (!this->_socket.reset(new_socket_fd))
    {
        int reset_error;

        reset_error = this->_socket.get_error();
        nw_close(new_socket_fd);
        freeaddrinfo(address_info);
        this->set_error(reset_error);
        return (1);
    }
    result = nw_connect(this->_socket.get(), address_info->ai_addr, address_info->ai_addrlen);
    freeaddrinfo(address_info);
    if (result < 0)
    {
        connect_error = ft_errno;
        this->close();
        this->set_error(connect_error);
        return (1);
    }
    if (this->perform_handshake(host, path) != 0)
    {
        int handshake_error;

        handshake_error = this->_error_code;
        this->close();
        this->set_error(handshake_error);
        return (1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_websocket_client::send_pong(const unsigned char *payload, std::size_t length)
{
    ft_string frame;
    uint32_t mask_value;
    unsigned char mask_key[4];
    std::size_t index_value;
    int socket_fd;

    socket_fd = this->_socket.get();
    if (socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }

    mask_value = ft_random_uint32();
    index_value = 0;
    while (index_value < 4)
    {
        mask_key[index_value] = static_cast<unsigned char>(mask_value >> (index_value * 8));
        index_value++;
    }
    frame.append(static_cast<char>(0x8A));
    if (length <= 125)
        frame.append(static_cast<char>(0x80 | static_cast<unsigned char>(length)));
    else if (length <= 65535)
    {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    }
    else
    {
        frame.append(static_cast<char>(0x80 | 127));
        index_value = 0;
        while (index_value < 8)
        {
            frame.append(static_cast<char>((length >> ((7 - index_value) * 8)) & 0xFF));
            index_value++;
        }
    }
    index_value = 0;
    while (index_value < 4)
    {
        frame.append(static_cast<char>(mask_key[index_value]));
        index_value++;
    }
    index_value = 0;
    while (index_value < length)
    {
        char masked_char = static_cast<char>(payload[index_value] ^ mask_key[index_value % 4]);
        frame.append(masked_char);
        index_value++;
    }
    if (nw_send(socket_fd, frame.c_str(), frame.size(), 0) < 0)
    {
        this->set_error(ft_errno);
        return (1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_websocket_client::send_text(const ft_string &message)
{
    ft_string frame;
    uint32_t mask_value;
    unsigned char mask_key[4];
    std::size_t index_value;
    std::size_t length;
    int socket_fd;

    socket_fd = this->_socket.get();
    if (socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }

    mask_value = ft_random_uint32();
    index_value = 0;
    while (index_value < 4)
    {
        mask_key[index_value] = static_cast<unsigned char>(mask_value >> (index_value * 8));
        index_value++;
    }
    frame.append(static_cast<char>(0x81));
    length = message.size();
    const char *message_data = message.c_str();
    if (length <= 125)
        frame.append(static_cast<char>(0x80 | static_cast<unsigned char>(length)));
    else if (length <= 65535)
    {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    }
    else
    {
        frame.append(static_cast<char>(0x80 | 127));
        index_value = 0;
        while (index_value < 8)
        {
            frame.append(static_cast<char>((length >> ((7 - index_value) * 8)) & 0xFF));
            index_value++;
        }
    }
    index_value = 0;
    while (index_value < 4)
    {
        frame.append(static_cast<char>(mask_key[index_value]));
        index_value++;
    }
    index_value = 0;
    while (index_value < length)
    {
        char masked_char = static_cast<char>(message_data[index_value] ^ mask_key[index_value % 4]);
        frame.append(masked_char);
        index_value++;
    }
    if (nw_send(socket_fd, frame.c_str(), frame.size(), 0) < 0)
    {
        this->set_error(ft_errno);
        return (1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_websocket_client::receive_text(ft_string &message)
{
    unsigned char header[2];
    unsigned char mask_key[4];
    bool mask_bit_set;
    std::size_t payload_length;
    ssize_t bytes_received;
    unsigned char *payload;
    std::size_t index_value;
    unsigned char opcode;
    int socket_fd;

    message.clear();
    socket_fd = this->_socket.get();
    if (socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    while (true)
    {
        bytes_received = nw_recv(socket_fd, header, 2, 0);
        if (bytes_received <= 0)
        {
            this->set_error(ft_errno);
            return (1);
        }
        opcode = header[0] & 0x0F;
        mask_bit_set = (header[1] & 0x80) != 0;
        payload_length = static_cast<std::size_t>(header[1] & 0x7F);
        if (payload_length == 126)
        {
            unsigned char extended[2];
            bytes_received = nw_recv(socket_fd, extended, 2, 0);
            if (bytes_received <= 0)
            {
                this->set_error(ft_errno);
                return (1);
            }
            payload_length = static_cast<std::size_t>((extended[0] << 8) | extended[1]);
        }
        else if (payload_length == 127)
        {
            unsigned char extended[8];
            std::size_t shift_index;
            bytes_received = nw_recv(socket_fd, extended, 8, 0);
            if (bytes_received <= 0)
            {
                this->set_error(ft_errno);
                return (1);
            }
            payload_length = 0;
            shift_index = 0;
            while (shift_index < 8)
            {
                payload_length = (payload_length << 8) | extended[shift_index];
                shift_index++;
            }
        }
        if (mask_bit_set)
        {
            bytes_received = nw_recv(socket_fd, mask_key, 4, 0);
            if (bytes_received <= 0)
            {
                this->set_error(ft_errno);
                return (1);
            }
        }
        payload = static_cast<unsigned char *>(cma_malloc(payload_length));
        if (!payload)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (1);
        }
        index_value = 0;
        while (index_value < payload_length)
        {
            bytes_received = nw_recv(socket_fd, payload + index_value, payload_length - index_value, 0);
            if (bytes_received <= 0)
            {
                cma_free(payload);
                this->set_error(ft_errno);
                return (1);
            }
            index_value += static_cast<std::size_t>(bytes_received);
        }
        if (mask_bit_set)
        {
            index_value = 0;
            while (index_value < payload_length)
            {
                payload[index_value] = static_cast<unsigned char>(payload[index_value] ^ mask_key[index_value % 4]);
                index_value++;
            }
        }
        if (opcode == 0x9)
        {
            this->send_pong(payload, payload_length);
            cma_free(payload);
            continue ;
        }
        if (opcode == 0xA)
        {
            cma_free(payload);
            continue ;
        }
        if (opcode == 0x1)
        {
            message.clear();
            index_value = 0;
            while (index_value < payload_length)
            {
                message.append(static_cast<char>(payload[index_value]));
                index_value++;
            }
            cma_free(payload);
            this->set_error(ER_SUCCESS);
            return (0);
        }
        cma_free(payload);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
}

int ft_websocket_client::get_error() const
{
    return (this->_error_code);
}

const char *ft_websocket_client::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
