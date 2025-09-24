#include "websocket_server.hpp"
#include "networking.hpp"
#include "socket_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../Encryption/basic_encryption.hpp"
#include "../Encryption/encryption_sha1.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>

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

ft_websocket_server::ft_websocket_server() : _server_socket(ft_nullptr), _error_code(ER_SUCCESS)
{
    return ;
}

ft_websocket_server::~ft_websocket_server()
{
    if (this->_server_socket)
    {
        this->_server_socket->close_socket();
        delete this->_server_socket;
        this->_server_socket = ft_nullptr;
    }
    return ;
}

void ft_websocket_server::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_websocket_server::start(const char *ip, uint16_t port, int address_family, bool non_blocking)
{
    SocketConfig configuration;

    configuration._type = SocketType::SERVER;
    configuration._ip = ip;
    configuration._port = port;
    configuration._address_family = address_family;
    configuration._non_blocking = non_blocking;
    configuration._recv_timeout = 5000;
    configuration._send_timeout = 5000;
    if (this->_server_socket)
    {
        delete this->_server_socket;
        this->_server_socket = ft_nullptr;
    }
    this->_server_socket = new ft_socket(configuration);
    if (!this->_server_socket || this->_server_socket->get_error() != ER_SUCCESS)
    {
        if (this->_server_socket)
        {
            this->set_error(this->_server_socket->get_error());
        }
        else
        {
            this->set_error(FT_EINVAL);
        }
        return (1);
    }
    this->_error_code = ER_SUCCESS;
    return (0);
}

int ft_websocket_server::perform_handshake(int client_fd)
{
    char buffer[1024];
    ssize_t bytes_received;
    ft_string request;
    const char *key_line;
    const char *line_end;
    ft_string key;
    ft_string accept;
    ft_string response;

    bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    buffer[bytes_received] = '\0';
    request = buffer;
    key_line = ft_strstr(request.c_str(), "Sec-WebSocket-Key: ");
    if (!key_line)
    {
        this->set_error(FT_EINVAL);
        return (1);
    }
    key_line += ft_strlen("Sec-WebSocket-Key: ");
    line_end = ft_strstr(key_line, "\r\n");
    key.clear();
    if (line_end)
    {
        std::size_t index_value = 0;
        while (key_line + index_value < line_end)
        {
            key.append(key_line[index_value]);
            index_value++;
        }
    }
    else
    {
        std::size_t index_value = 0;
        while (key_line[index_value] != '\0')
        {
            key.append(key_line[index_value]);
            index_value++;
        }
    }
    compute_accept_key(key, accept);
    response.append("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ");
    response.append(accept);
    response.append("\r\n\r\n");
    nw_send(client_fd, response.c_str(), response.size(), 0);
    return (0);
}

int ft_websocket_server::send_pong(int client_fd, const unsigned char *payload, std::size_t length)
{
    ft_string frame;
    std::size_t index_value;

    frame.append(static_cast<char>(0x8A));
    if (length <= 125)
        frame.append(static_cast<char>(static_cast<unsigned char>(length)));
    else if (length <= 65535)
    {
        frame.append(static_cast<char>(126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    }
    else
    {
        frame.append(static_cast<char>(127));
        index_value = 0;
        while (index_value < 8)
        {
            frame.append(static_cast<char>((length >> ((7 - index_value) * 8)) & 0xFF));
            index_value++;
        }
    }
    index_value = 0;
    while (index_value < length)
    {
        frame.append(static_cast<char>(payload[index_value]));
        index_value++;
    }
    nw_send(client_fd, frame.c_str(), frame.size(), 0);
    return (0);
}

int ft_websocket_server::receive_frame(int client_fd, ft_string &message)
{
    unsigned char header[2];
    unsigned char mask_key[4];
    std::size_t payload_length;
    ssize_t bytes_received;
    unsigned char *payload;
    std::size_t index_value;
    unsigned char opcode;

    message.clear();
    while (true)
    {
        bytes_received = nw_recv(client_fd, header, 2, 0);
        if (bytes_received <= 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            return (1);
        }
        opcode = header[0] & 0x0F;
        payload_length = static_cast<std::size_t>(header[1] & 0x7F);
        if (payload_length == 126)
        {
            unsigned char extended[2];
            bytes_received = nw_recv(client_fd, extended, 2, 0);
            if (bytes_received <= 0)
            {
                this->set_error(errno + ERRNO_OFFSET);
                return (1);
            }
            payload_length = static_cast<std::size_t>((extended[0] << 8) | extended[1]);
        }
        else if (payload_length == 127)
        {
            unsigned char extended[8];
            std::size_t shift_index;
            bytes_received = nw_recv(client_fd, extended, 8, 0);
            if (bytes_received <= 0)
            {
                this->set_error(errno + ERRNO_OFFSET);
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
        bytes_received = nw_recv(client_fd, mask_key, 4, 0);
        if (bytes_received <= 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            return (1);
        }
        payload = static_cast<unsigned char *>(cma_malloc(payload_length));
        if (!payload)
        {
            this->set_error(FT_EINVAL);
            return (1);
        }
        index_value = 0;
        while (index_value < payload_length)
        {
            bytes_received = nw_recv(client_fd, payload + index_value, payload_length - index_value, 0);
            if (bytes_received <= 0)
            {
                cma_free(payload);
                this->set_error(errno + ERRNO_OFFSET);
                return (1);
            }
            index_value += static_cast<std::size_t>(bytes_received);
        }
        index_value = 0;
        while (index_value < payload_length)
        {
            payload[index_value] = static_cast<unsigned char>(payload[index_value] ^ mask_key[index_value % 4]);
            index_value++;
        }
        if (opcode == 0x9)
        {
            this->send_pong(client_fd, payload, payload_length);
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
            this->_error_code = ER_SUCCESS;
            return (0);
        }
        cma_free(payload);
        this->set_error(FT_EINVAL);
        return (1);
    }
}

int ft_websocket_server::run_once(int &client_fd, ft_string &message)
{
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int result;

    address_length = sizeof(client_address);
    client_fd = nw_accept(this->_server_socket->get_fd(), reinterpret_cast<struct sockaddr*>(&client_address), &address_length);
    if (client_fd < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    if (this->perform_handshake(client_fd) != 0)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (1);
    }
    result = this->receive_frame(client_fd, message);
    if (result != 0)
    {
        FT_CLOSE_SOCKET(client_fd);
        return (1);
    }
    this->_error_code = ER_SUCCESS;
    return (0);
}

int ft_websocket_server::send_text(int client_fd, const ft_string &message)
{
    ft_string frame;
    std::size_t length;
    std::size_t index_value;

    if (client_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (1);
    }
    frame.append(static_cast<char>(0x81));
    length = message.size();
    if (length <= 125)
    {
        frame.append(static_cast<char>(length));
    }
    else if (length <= 65535)
    {
        frame.append(static_cast<char>(126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    }
    else
    {
        frame.append(static_cast<char>(127));
        index_value = 0;
        while (index_value < 8)
        {
            frame.append(static_cast<char>((length >> ((7 - index_value) * 8)) & 0xFF));
            index_value++;
        }
    }
    index_value = 0;
    while (index_value < length)
    {
        frame.append(message.c_str()[index_value]);
        index_value++;
    }
    if (nw_send(client_fd, frame.c_str(), frame.size(), 0) < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    this->_error_code = ER_SUCCESS;
    return (0);
}

int ft_websocket_server::get_error() const
{
    return (this->_error_code);
}

const char *ft_websocket_server::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
