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
#include <zlib.h>

static char websocket_ascii_lower(char character)
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character + 32));
    return (character);
}

static int websocket_case_insensitive_compare(const char *left, const char *right, std::size_t length)
{
    std::size_t index_value;

    index_value = 0;
    while (index_value < length)
    {
        unsigned char left_value;
        unsigned char right_value;

        left_value = static_cast<unsigned char>(websocket_ascii_lower(left[index_value]));
        right_value = static_cast<unsigned char>(websocket_ascii_lower(right[index_value]));
        if (left_value != right_value)
            return (static_cast<int>(left_value) - static_cast<int>(right_value));
        index_value++;
    }
    return (0);
}

static const char *websocket_find_header_value(const ft_string &request,
    const char *header_name, std::size_t &value_length)
{
    const char  *cursor;
    const char  *line_end;
    std::size_t header_length;
    std::size_t line_length;
    std::size_t offset;

    cursor = request.c_str();
    header_length = ft_strlen(header_name);
    while (*cursor != '\0')
    {
        line_end = ft_strstr(cursor, "\r\n");
        if (line_end != ft_nullptr)
            line_length = static_cast<std::size_t>(line_end - cursor);
        else
            line_length = ft_strlen(cursor);
        if (line_length >= header_length)
        {
            if (websocket_case_insensitive_compare(cursor, header_name, header_length) == 0)
            {
                offset = header_length;
                while (offset < line_length && (cursor[offset] == ' ' || cursor[offset] == '\t'))
                    offset++;
                value_length = line_length - offset;
                return (cursor + offset);
            }
        }
        if (line_end == ft_nullptr)
            break ;
        cursor = line_end + 2;
    }
    value_length = 0;
    return (ft_nullptr);
}

static bool websocket_header_contains_token(const ft_string &request,
    const char *header_name, const char *token)
{
    std::size_t value_length;
    const char  *value_start;
    std::size_t token_length;
    std::size_t index_value;

    value_start = websocket_find_header_value(request, header_name, value_length);
    if (value_start == ft_nullptr || value_length == 0)
        return (false);
    token_length = ft_strlen(token);
    if (token_length == 0)
        return (false);
    if (token_length > value_length)
        return (false);
    index_value = 0;
    while (index_value + token_length <= value_length)
    {
        if (websocket_case_insensitive_compare(value_start + index_value, token, token_length) == 0)
        {
            if (index_value > 0)
            {
                char previous_character;

                previous_character = value_start[index_value - 1];
                if (previous_character != ' ' && previous_character != '\t'
                    && previous_character != ';' && previous_character != ',')
                {
                    index_value++;
                    continue ;
                }
            }
            if (index_value + token_length < value_length)
            {
                char next_character;

                next_character = value_start[index_value + token_length];
                if (next_character != ' ' && next_character != '\t'
                    && next_character != ';' && next_character != ',' && next_character != '\0')
                {
                    index_value++;
                    continue ;
                }
            }
            return (true);
        }
        index_value++;
    }
    return (false);
}

static int websocket_append_bytes(ft_vector<unsigned char> &buffer,
    const unsigned char *data, std::size_t length)
{
    std::size_t index_value;

    if (data == ft_nullptr && length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    index_value = 0;
    while (index_value < length)
    {
        buffer.push_back(data[index_value]);
        if (buffer.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = buffer.get_error();
            return (1);
        }
        index_value++;
    }
    return (0);
}

static int websocket_permessage_deflate_inflate(const unsigned char *payload,
    std::size_t payload_length, ft_string &message)
{
    unsigned char tail_bytes[4];
    ft_vector<unsigned char> input_buffer;
    ft_vector<unsigned char> output_buffer;
    z_stream stream;
    int zlib_result;

    if (payload == ft_nullptr && payload_length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    tail_bytes[0] = 0x00;
    tail_bytes[1] = 0x00;
    tail_bytes[2] = 0xFF;
    tail_bytes[3] = 0xFF;
    input_buffer.reserve(payload_length + 4);
    if (input_buffer.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = input_buffer.get_error();
        return (1);
    }
    if (websocket_append_bytes(input_buffer, payload, payload_length) != 0)
        return (1);
    if (websocket_append_bytes(input_buffer, tail_bytes, 4) != 0)
        return (1);
    ft_memset(&stream, 0, sizeof(stream));
    stream.next_in = input_buffer.begin();
    stream.avail_in = static_cast<uInt>(input_buffer.size());
    zlib_result = inflateInit2(&stream, -15);
    if (zlib_result != Z_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    while (true)
    {
        unsigned char output_chunk[512];
        std::size_t produced_bytes;

        stream.next_out = output_chunk;
        stream.avail_out = sizeof(output_chunk);
        zlib_result = inflate(&stream, Z_SYNC_FLUSH);
        produced_bytes = sizeof(output_chunk) - stream.avail_out;
        if (produced_bytes > 0)
        {
            if (websocket_append_bytes(output_buffer, output_chunk, produced_bytes) != 0)
            {
                inflateEnd(&stream);
                return (1);
            }
        }
        if (zlib_result == Z_STREAM_END)
            break ;
        if (zlib_result == Z_BUF_ERROR && stream.avail_in == 0)
            break ;
        if (zlib_result != Z_OK && zlib_result != Z_BUF_ERROR)
        {
            inflateEnd(&stream);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    inflateEnd(&stream);
    message.assign(reinterpret_cast<const char *>(output_buffer.begin()), output_buffer.size());
    if (message.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = message.get_error();
        return (1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int websocket_permessage_deflate_deflate(const ft_string &message,
    ft_vector<unsigned char> &compressed)
{
    z_stream stream;
    int zlib_result;
    ft_vector<unsigned char> output_buffer;

    ft_memset(&stream, 0, sizeof(stream));
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(message.c_str()));
    stream.avail_in = static_cast<uInt>(message.size());
    zlib_result = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (zlib_result != Z_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    while (true)
    {
        unsigned char output_chunk[512];
        std::size_t produced_bytes;

        stream.next_out = output_chunk;
        stream.avail_out = sizeof(output_chunk);
        zlib_result = deflate(&stream, Z_SYNC_FLUSH);
        produced_bytes = sizeof(output_chunk) - stream.avail_out;
        if (produced_bytes > 0)
        {
            if (websocket_append_bytes(output_buffer, output_chunk, produced_bytes) != 0)
            {
                deflateEnd(&stream);
                return (1);
            }
        }
        if (zlib_result == Z_STREAM_END)
            break ;
        if (stream.avail_in == 0 && stream.avail_out != 0)
            break ;
        if (zlib_result != Z_OK)
        {
            deflateEnd(&stream);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    deflateEnd(&stream);
    if (output_buffer.size() < 4)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    std::size_t trim_index;

    trim_index = 0;
    while (trim_index < 4)
    {
        output_buffer.pop_back();
        if (output_buffer.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output_buffer.get_error();
            return (1);
        }
        trim_index++;
    }
    compressed = ft_move(output_buffer);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

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

void ft_websocket_server::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != FT_ERR_SUCCESSS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

ft_websocket_server::ft_websocket_server()
    : _server_socket(ft_nullptr), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    this->_server_socket = ft_nullptr;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return ;
}

ft_websocket_server::~ft_websocket_server()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (this->_server_socket)
    {
        this->_server_socket->close_socket();
        delete this->_server_socket;
        this->_server_socket = ft_nullptr;
    }
    this->_connection_states.clear();
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return ;
}

void ft_websocket_server::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void ft_websocket_server::store_connection_state_locked(int client_fd,
    bool permessage_deflate_enabled)
{
    s_connection_state state;

    state._permessage_deflate_enabled = permessage_deflate_enabled;
    this->_connection_states[client_fd] = state;
    return ;
}

void ft_websocket_server::remove_connection_state_locked(int client_fd)
{
    std::map<int, s_connection_state>::iterator iterator_value;

    iterator_value = this->_connection_states.find(client_fd);
    if (iterator_value != this->_connection_states.end())
        this->_connection_states.erase(iterator_value);
    return ;
}

bool ft_websocket_server::connection_supports_permessage_deflate_locked(int client_fd) const
{
    std::map<int, s_connection_state>::const_iterator iterator_value;

    iterator_value = this->_connection_states.find(client_fd);
    if (iterator_value == this->_connection_states.end())
        return (false);
    return (iterator_value->second._permessage_deflate_enabled);
}

int ft_websocket_server::start(const char *ip, uint16_t port, int address_family, bool non_blocking)
{
    int entry_errno;

    entry_errno = ft_errno;
    SocketConfig configuration;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    configuration._type = SocketType::SERVER;
    configuration._ip = ip;
    configuration._port = port;
    configuration._address_family = address_family;
    configuration._non_blocking = non_blocking;
    configuration._recv_timeout = 5000;
    configuration._send_timeout = 5000;
    if (this->_server_socket)
    {
        this->_server_socket->close_socket();
        delete this->_server_socket;
        this->_server_socket = ft_nullptr;
    }
    this->_server_socket = new ft_socket(configuration);
    if (!this->_server_socket || this->_server_socket->get_error() != FT_ERR_SUCCESSS)
    {
        int error_code;

        if (this->_server_socket)
        {
            error_code = this->_server_socket->get_error();
            delete this->_server_socket;
            this->_server_socket = ft_nullptr;
        }
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    this->_connection_states.clear();
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (0);
}

int ft_websocket_server::perform_handshake_locked(int client_fd, ft_unique_lock<pt_mutex> &guard)
{
    static const std::size_t MAX_HANDSHAKE_SIZE = 8192;
    char buffer[1024];
    ssize_t bytes_received;
    ft_string request;
    const char *key_line;
    const char *line_end;
    ft_string key;
    ft_string accept;
    ft_string response;
    std::size_t request_size;
    bool permessage_deflate_enabled;

    (void)guard;
    this->remove_connection_state_locked(client_fd);
    request.clear();
    while (true)
    {
        if (request.size() >= MAX_HANDSHAKE_SIZE)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (1);
        }
        bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
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
        request_size = request.size();
        if (request_size + static_cast<std::size_t>(bytes_received) > MAX_HANDSHAKE_SIZE)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (1);
        }
        request.append(buffer);
        if (ft_strstr(request.c_str(), "\r\n\r\n"))
            break;
    }
    key_line = ft_strstr(request.c_str(), "Sec-WebSocket-Key: ");
    if (!key_line)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    key_line += ft_strlen("Sec-WebSocket-Key: ");
    line_end = ft_strstr(key_line, "\r\n");
    key.clear();
    if (line_end)
    {
        std::size_t index_value;

        index_value = 0;
        while (key_line + index_value < line_end)
        {
            key.append(key_line[index_value]);
            index_value++;
        }
    }
    else
    {
        std::size_t index_value;

        index_value = 0;
        while (key_line[index_value] != '\0')
        {
            key.append(key_line[index_value]);
            index_value++;
        }
    }
    compute_accept_key(key, accept);
    permessage_deflate_enabled = websocket_header_contains_token(request,
        "Sec-WebSocket-Extensions:", "permessage-deflate");
    response.clear();
    response.append("HTTP/1.1 101 Switching Protocols\r\n");
    response.append("Upgrade: websocket\r\n");
    response.append("Connection: Upgrade\r\n");
    response.append("Sec-WebSocket-Accept: ");
    response.append(accept);
    response.append("\r\n");
    if (permessage_deflate_enabled != false)
    {
        response.append("Sec-WebSocket-Extensions: permessage-deflate; server_no_context_takeover; client_no_context_takeover\r\n");
    }
    response.append("\r\n");
    if (nw_send(client_fd, response.c_str(), response.size(), 0) < 0)
    {
        this->set_error(ft_errno);
        this->remove_connection_state_locked(client_fd);
        return (1);
    }
    this->store_connection_state_locked(client_fd, permessage_deflate_enabled);
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_websocket_server::send_pong_locked(int client_fd, const unsigned char *payload, std::size_t length, ft_unique_lock<pt_mutex> &guard)
{
    ft_string frame;
    std::size_t index_value;

    (void)guard;
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
    if (nw_send(client_fd, frame.c_str(), frame.size(), 0) < 0)
    {
        this->set_error(ft_errno);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_websocket_server::receive_frame_locked(int client_fd, ft_string &message, ft_unique_lock<pt_mutex> &guard)
{
    unsigned char header[2];
    unsigned char mask_key[4];
    std::size_t payload_length;
    ssize_t bytes_received;
    unsigned char *payload;
    std::size_t index_value;
    unsigned char opcode;
    unsigned char rsv_bits;
    bool permessage_deflate_enabled;
    bool is_final;
    bool masked;
    bool is_control_frame;

    message.clear();
    permessage_deflate_enabled = this->connection_supports_permessage_deflate_locked(client_fd);
    while (true)
    {
        bytes_received = nw_recv(client_fd, header, 2, 0);
        if (bytes_received <= 0)
        {
            this->set_error(ft_errno);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        opcode = header[0] & 0x0F;
        rsv_bits = header[0] & 0x70;
        is_final = ((header[0] & 0x80) != 0);
        masked = ((header[1] & 0x80) != 0);
        payload_length = static_cast<std::size_t>(header[1] & 0x7F);
        is_control_frame = ((opcode & 0x08) != 0);
        if (is_final == false)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if ((rsv_bits & 0x20) != 0 || (rsv_bits & 0x10) != 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if ((rsv_bits & 0x40) != 0 && permessage_deflate_enabled == false)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if (is_control_frame != false && rsv_bits != 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if (payload_length == 126)
        {
            unsigned char extended[2];

            bytes_received = nw_recv(client_fd, extended, 2, 0);
            if (bytes_received <= 0)
            {
                this->set_error(ft_errno);
                this->remove_connection_state_locked(client_fd);
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
                this->set_error(ft_errno);
                this->remove_connection_state_locked(client_fd);
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
        if (is_control_frame != false && payload_length > 125)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if (payload_length > compression_max_size)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if (masked == false)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        bytes_received = nw_recv(client_fd, mask_key, 4, 0);
        if (bytes_received <= 0)
        {
            this->set_error(ft_errno);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        if (payload_length > 0)
            payload = static_cast<unsigned char *>(cma_malloc(payload_length));
        else
            payload = ft_nullptr;
        if (payload_length > 0 && payload == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->remove_connection_state_locked(client_fd);
            return (1);
        }
        index_value = 0;
        while (index_value < payload_length)
        {
            bytes_received = nw_recv(client_fd, payload + index_value, payload_length - index_value, 0);
            if (bytes_received <= 0)
            {
                if (payload != ft_nullptr)
                    cma_free(payload);
                this->set_error(ft_errno);
                this->remove_connection_state_locked(client_fd);
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
            if (this->send_pong_locked(client_fd, payload, payload_length, guard) != 0)
            {
                if (payload != ft_nullptr)
                    cma_free(payload);
                this->remove_connection_state_locked(client_fd);
                return (1);
            }
            if (payload != ft_nullptr)
                cma_free(payload);
            continue ;
        }
        if (opcode == 0xA)
        {
            if (payload != ft_nullptr)
                cma_free(payload);
            continue ;
        }
        if (opcode == 0x1)
        {
            if ((rsv_bits & 0x40) != 0)
            {
                if (websocket_permessage_deflate_inflate(payload, payload_length, message) != 0)
                {
                    if (payload != ft_nullptr)
                        cma_free(payload);
                    this->remove_connection_state_locked(client_fd);
                    return (1);
                }
            }
            else
            {
                message.clear();
                index_value = 0;
                while (index_value < payload_length)
                {
                    message.append(static_cast<char>(payload[index_value]));
                    index_value++;
                }
                if (message.get_error() != FT_ERR_SUCCESSS)
                {
                    if (payload != ft_nullptr)
                        cma_free(payload);
                    this->set_error(message.get_error());
                    this->remove_connection_state_locked(client_fd);
                    return (1);
                }
            }
            if (payload != ft_nullptr)
                cma_free(payload);
            this->set_error(FT_ERR_SUCCESSS);
            return (0);
        }
        if (payload != ft_nullptr)
            cma_free(payload);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->remove_connection_state_locked(client_fd);
        return (1);
    }
}

int ft_websocket_server::run_once(int &client_fd, ft_string &message)
{
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    if (this->_server_socket == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    address_length = sizeof(client_address);
    client_fd = nw_accept(this->_server_socket->get_fd(), reinterpret_cast<struct sockaddr*>(&client_address), &address_length);
    if (client_fd < 0)
    {
        this->set_error(ft_errno);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    if (this->perform_handshake_locked(client_fd, guard) != 0)
    {
        int handshake_error;

        handshake_error = this->_error_code;
        nw_close(client_fd);
        this->set_error(handshake_error);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    result = this->receive_frame_locked(client_fd, message, guard);
    if (result != 0)
    {
        int receive_error;

        receive_error = this->_error_code;
        nw_close(client_fd);
        this->set_error(receive_error);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (0);
}

int ft_websocket_server::send_text(int client_fd, const ft_string &message)
{
    ft_string frame;
    std::size_t length;
    std::size_t index_value;
    int entry_errno;
    bool permessage_deflate_enabled;
    ft_vector<unsigned char> compressed_payload;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    if (client_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    permessage_deflate_enabled = this->connection_supports_permessage_deflate_locked(client_fd);
    if (permessage_deflate_enabled != false)
    {
        if (websocket_permessage_deflate_deflate(message, compressed_payload) != 0)
        {
            this->set_error(ft_errno);
            ft_websocket_server::restore_errno(guard, entry_errno);
            return (1);
        }
        frame.append(static_cast<char>(0xC1));
        length = compressed_payload.size();
    }
    else
    {
        frame.append(static_cast<char>(0x81));
        length = message.size();
    }
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
    if (permessage_deflate_enabled != false)
    {
        while (index_value < length)
        {
            frame.append(static_cast<char>(compressed_payload[index_value]));
            index_value++;
        }
    }
    else
    {
        while (index_value < length)
        {
            frame.append(message.c_str()[index_value]);
            index_value++;
        }
    }
    if (nw_send(client_fd, frame.c_str(), frame.size(), 0) < 0)
    {
        this->set_error(ft_errno);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (0);
}

int ft_websocket_server::get_port(unsigned short &port_value) const
{
    struct sockaddr_storage local_address;
    socklen_t address_length;
    int server_fd;
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    port_value = 0;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_websocket_server *>(this)->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    if (this->_server_socket == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    server_fd = this->_server_socket->get_fd();
    if (server_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    address_length = sizeof(local_address);
    if (getsockname(server_fd, reinterpret_cast<struct sockaddr*>(&local_address), &address_length) != 0)
    {
        this->set_error(ft_set_errno_from_system_error(errno));
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    if (local_address.ss_family == AF_INET)
    {
        const struct sockaddr_in *ipv4_address;

        ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&local_address);
        port_value = ntohs(ipv4_address->sin_port);
    }
    else if (local_address.ss_family == AF_INET6)
    {
        const struct sockaddr_in6 *ipv6_address;

        ipv6_address = reinterpret_cast<const struct sockaddr_in6*>(&local_address);
        port_value = ntohs(ipv6_address->sin6_port);
    }
    else
    {
        this->set_error(FT_ERR_INVALID_STATE);
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (0);
}

int ft_websocket_server::get_error() const
{
    int entry_errno;
    int error_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_websocket_server *>(this)->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (const_cast<ft_websocket_server *>(this)->_error_code);
    }
    error_value = this->_error_code;
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_websocket_server::get_error_str() const
{
    int entry_errno;
    const char *error_string;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_websocket_server *>(this)->set_error(guard.get_error());
        ft_websocket_server::restore_errno(guard, entry_errno);
        return (ft_strerror(const_cast<ft_websocket_server *>(this)->_error_code));
    }
    error_string = ft_strerror(this->_error_code);
    ft_websocket_server::restore_errno(guard, entry_errno);
    return (error_string);
}
