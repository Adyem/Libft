#include "websocket_client.hpp"
#include "networking.hpp"
#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../RNG/rng.hpp"
#include "../Encryption/encryption.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstring>
#include <cstdio>
#include <new>

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"
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
    int32_t initialize_error;

    initialize_error = magic.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return ;
    magic = key;
    magic.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    sha1_hash(magic.c_str(), magic.size(), digest);
    accept.clear();
    encoded = ft_base64_encode(digest, 20, &encoded_size);
    if (encoded != ft_nullptr)
    {
        ft_size_t index_value;

        index_value = 0;
        while (index_value < encoded_size)
        {
            accept.append(reinterpret_cast<char *>(encoded)[index_value]);
            index_value++;
        }
        cma_free(encoded);
    }
    return ;
}

ft_websocket_client::ft_websocket_client() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _socket(), _mutex(ft_nullptr)
{
    return ;
}

ft_websocket_client::~ft_websocket_client() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t ft_websocket_client::move(ft_websocket_client &other) noexcept
{
    return (this->initialize(static_cast<ft_websocket_client &&>(other)));
}

int32_t ft_websocket_client::initialize(const ft_websocket_client &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_websocket_client::initialize(const ft_websocket_client &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    return (this->initialize());
}

int32_t ft_websocket_client::initialize(ft_websocket_client &&other) noexcept
{
    int32_t initialize_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "ft_websocket_client::initialize", "initialize called on initialised instance");
    if (this->_socket.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    (void)this->close_locked();
    (void)this->_socket.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::enable_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    this->_mutex = new (std::nothrow) pt_recursive_mutex();
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (this->_mutex->initialize() != FT_ERR_SUCCESS)
    {
        delete this->_mutex;
        this->_mutex = ft_nullptr;
        return (FT_ERR_INVALID_OPERATION);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::disable_thread_safety() noexcept
{
    int32_t destroy_error;
    pt_recursive_mutex *mutex_pointer;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_error != FT_ERR_SUCCESS && destroy_error != FT_ERR_INVALID_STATE)
        return (destroy_error);
    return (FT_ERR_SUCCESS);
}

ft_bool ft_websocket_client::is_thread_safe() const noexcept
{
    if (this->_mutex != ft_nullptr)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t ft_websocket_client::close_locked()
{
    if (!this->_socket.close())
        return (FT_ERR_INVALID_OPERATION);
    return (FT_ERR_SUCCESS);
}

void ft_websocket_client::close()
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_websocket_client::close");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    (void)this->close_locked();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_websocket_client::perform_handshake_locked(const char *host, const char *path)
{
    ft_size_t byte_index;
    ft_size_t shift_index;
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
    int32_t socket_fd;
    const char *request_data;
    ft_size_t total_sent;
    ssize_t send_result;
    int32_t string_init_error;

    string_init_error = key_string.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (string_init_error);
    string_init_error = request.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (string_init_error);
    string_init_error = response.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (string_init_error);
    string_init_error = accept_key.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (string_init_error);
    string_init_error = expected.initialize();
    if (string_init_error != FT_ERR_SUCCESS)
        return (string_init_error);
    socket_fd = this->_socket.get();
    if (socket_fd < 0)
        return (FT_ERR_INVALID_OPERATION);
    byte_index = 0;
    while (byte_index < 16)
    {
        uint32_t random_value;

        random_value = ft_random_uint32();
        shift_index = 0;
        while (shift_index < 4 && byte_index < 16)
        {
            random_key[byte_index] = static_cast<unsigned char>(random_value >> (shift_index * 8));
            byte_index++;
            shift_index++;
        }
    }
    encoded_key = ft_base64_encode(random_key, 16, &encoded_size);
    if (encoded_key == ft_nullptr)
        return (FT_ERR_INVALID_OPERATION);
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
    request_data = request.c_str();
    total_sent = 0;
    while (total_sent < request.size())
    {
        send_result = nw_send(socket_fd, request_data + total_sent, request.size() - total_sent, 0);
        if (send_result <= 0)
            return (FT_ERR_INVALID_OPERATION);
        total_sent += static_cast<ft_size_t>(send_result);
    }
    response.clear();
    while (FT_TRUE)
    {
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            return (FT_ERR_INVALID_OPERATION);
        buffer[bytes_received] = '\0';
        response.append(buffer);
        if (ft_strstr(response.c_str(), "\r\n\r\n") != ft_nullptr)
            break ;
    }
    accept_line = ft_strstr(response.c_str(), "Sec-WebSocket-Accept: ");
    if (accept_line == ft_nullptr)
        return (FT_ERR_INVALID_OPERATION);
    accept_line += ft_strlen("Sec-WebSocket-Accept: ");
    line_end = ft_strstr(accept_line, "\r\n");
    accept_key.clear();
    if (line_end != ft_nullptr)
    {
        byte_index = 0;
        while (accept_line + byte_index < line_end)
        {
            accept_key.append(accept_line[byte_index]);
            byte_index++;
        }
    }
    else
        accept_key.append(accept_line);
    compute_accept_key(key_string, expected);
    if (!(accept_key == expected))
        return (FT_ERR_INVALID_OPERATION);
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::connect(const char *host, uint16_t port, const char *path)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    char port_string[8];
    int32_t result;
    int32_t new_socket_fd;
    int32_t lock_error;

    if (host == ft_nullptr || path == ft_nullptr)
        return (FT_ERR_INVALID_OPERATION);
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_websocket_client::connect");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    ft_memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    std::snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host, port_string, &address_hints, &address_info) != 0)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_OPERATION);
    }
    new_socket_fd = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (new_socket_fd < 0)
    {
        freeaddrinfo(address_info);
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_OPERATION);
    }
    if (!this->_socket.reset(new_socket_fd))
    {
        (void)nw_close(new_socket_fd);
        freeaddrinfo(address_info);
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_OPERATION);
    }
    result = nw_connect(this->_socket.get(), address_info->ai_addr,
            address_info->ai_addrlen);
    freeaddrinfo(address_info);
    if (result < 0)
    {
        (void)this->close_locked();
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_OPERATION);
    }
    if (this->perform_handshake_locked(host, path) != 0)
    {
        (void)this->close_locked();
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_OPERATION);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::send_pong_locked(const unsigned char *payload, ft_size_t length)
{
    ft_string frame;
    uint32_t mask_value;
    unsigned char mask_key[4];
    ft_size_t index_value;
    int32_t socket_fd;
    int32_t initialize_error;

    initialize_error = frame.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    socket_fd = this->_socket.get();
    if (socket_fd < 0)
        return (FT_ERR_INVALID_OPERATION);
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
        return (FT_ERR_INVALID_OPERATION);
    index_value = 0;
    while (index_value < 4)
    {
        frame.append(static_cast<char>(mask_key[index_value]));
        index_value++;
    }
    index_value = 0;
    while (index_value < length)
    {
        frame.append(static_cast<char>(payload[index_value] ^ mask_key[index_value % 4]));
        index_value++;
    }
    if (nw_send(socket_fd, frame.c_str(), frame.size(), 0) < 0)
        return (FT_ERR_INVALID_OPERATION);
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::send_text_locked(const ft_string &message)
{
    ft_string frame;
    uint32_t mask_value;
    unsigned char mask_key[4];
    ft_size_t index_value;
    ft_size_t length;
    int32_t socket_fd;
    const char *message_data;
    int32_t initialize_error;

    initialize_error = frame.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    socket_fd = this->_socket.get();
    if (socket_fd < 0)
        return (FT_ERR_INVALID_OPERATION);
    mask_value = ft_random_uint32();
    index_value = 0;
    while (index_value < 4)
    {
        mask_key[index_value] = static_cast<unsigned char>(mask_value >> (index_value * 8));
        index_value++;
    }
    frame.append(static_cast<char>(0x81));
    length = message.size();
    message_data = message.c_str();
    if (length <= 125)
        frame.append(static_cast<char>(0x80 | static_cast<unsigned char>(length)));
    else if (length <= 65535)
    {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(static_cast<char>(length & 0xFF));
    }
    else
        return (FT_ERR_INVALID_OPERATION);
    index_value = 0;
    while (index_value < 4)
    {
        frame.append(static_cast<char>(mask_key[index_value]));
        index_value++;
    }
    index_value = 0;
    while (index_value < length)
    {
        frame.append(static_cast<char>(message_data[index_value] ^ mask_key[index_value % 4]));
        index_value++;
    }
    if (nw_send(socket_fd, frame.c_str(), frame.size(), 0) < 0)
        return (FT_ERR_INVALID_OPERATION);
    return (FT_ERR_SUCCESS);
}

int32_t ft_websocket_client::send_text(const ft_string &message)
{
    int32_t lock_error;
    int32_t send_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_websocket_client::send_text");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    send_result = this->send_text_locked(message);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (send_result);
}

int32_t ft_websocket_client::receive_text_locked(ft_string &message)
{
    unsigned char header[2];
    unsigned char mask_key[4];
    ft_bool mask_bit_set;
    ft_size_t payload_length;
    ssize_t bytes_received;
    unsigned char *payload;
    ft_size_t index_value;
    unsigned char opcode;
    int32_t socket_fd;

    message.clear();
    socket_fd = this->_socket.get();
    if (socket_fd < 0)
        return (FT_ERR_INVALID_OPERATION);
    while (FT_TRUE)
    {
        bytes_received = nw_recv(socket_fd, header, 2, 0);
        if (bytes_received <= 0)
            return (FT_ERR_INVALID_OPERATION);
        opcode = header[0] & 0x0F;
        mask_bit_set = (header[1] & 0x80) != 0;
        payload_length = static_cast<ft_size_t>(header[1] & 0x7F);
        if (payload_length == 126)
        {
            unsigned char extended[2];

            bytes_received = nw_recv(socket_fd, extended, 2, 0);
            if (bytes_received <= 0)
                return (FT_ERR_INVALID_OPERATION);
            payload_length = static_cast<ft_size_t>((extended[0] << 8) | extended[1]);
        }
        else if (payload_length == 127)
            return (FT_ERR_INVALID_OPERATION);
        if (mask_bit_set)
        {
            bytes_received = nw_recv(socket_fd, mask_key, 4, 0);
            if (bytes_received <= 0)
                return (FT_ERR_INVALID_OPERATION);
        }
        payload = static_cast<unsigned char *>(cma_malloc(payload_length));
        if (payload == ft_nullptr)
            return (FT_ERR_INVALID_OPERATION);
        index_value = 0;
        while (index_value < payload_length)
        {
            bytes_received = nw_recv(socket_fd, payload + index_value, payload_length - index_value, 0);
            if (bytes_received <= 0)
            {
                cma_free(payload);
                return (FT_ERR_INVALID_OPERATION);
            }
            index_value += static_cast<ft_size_t>(bytes_received);
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
            if (this->send_pong_locked(payload, payload_length) != 0)
            {
                cma_free(payload);
                return (FT_ERR_INVALID_OPERATION);
            }
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
            index_value = 0;
            while (index_value < payload_length)
            {
                message.append(static_cast<char>(payload[index_value]));
                index_value++;
            }
            cma_free(payload);
            return (FT_ERR_SUCCESS);
        }
        cma_free(payload);
        return (FT_ERR_INVALID_OPERATION);
    }
}

int32_t ft_websocket_client::receive_text(ft_string &message)
{
    int32_t lock_error;
    int32_t receive_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_websocket_client::receive_text");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    receive_result = this->receive_text_locked(message);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (receive_result);
}
