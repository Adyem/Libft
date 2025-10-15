#include "http2_client.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <openssl/ssl.h>

static void http2_append_raw_byte(ft_string &target, unsigned char value)
{
    target.append(static_cast<char>(value));
    return ;
}

http2_stream_manager::http2_stream_manager() noexcept
    : _streams(), _error_code(ER_SUCCESS)
{
    return ;
}

http2_stream_manager::~http2_stream_manager() noexcept
{
    this->_streams.clear();
    this->set_error(ER_SUCCESS);
    return ;
}

void http2_stream_manager::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

bool http2_stream_manager::open_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, ft_string> *existing_entry;

    existing_entry = this->_streams.find(stream_identifier);
    if (existing_entry != ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    this->_streams.insert(stream_identifier, ft_string());
    if (this->_streams.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_streams.get_error());
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::append_data(uint32_t stream_identifier, const char *data,
    size_t length) noexcept
{
    Pair<uint32_t, ft_string> *stream_entry;
    size_t index;

    if (!data && length > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    index = 0;
    while (index < length)
    {
        stream_entry->value.append(data[index]);
        if (stream_entry->value.get_error() != ER_SUCCESS)
        {
            this->set_error(stream_entry->value.get_error());
            return (false);
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::close_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, ft_string> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    this->_streams.remove(stream_identifier);
    if (this->_streams.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_streams.get_error());
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::get_stream_buffer(uint32_t stream_identifier,
    ft_string &out_buffer) const noexcept
{
    const Pair<uint32_t, ft_string> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    out_buffer = stream_entry->value;
    if (out_buffer.get_error() != ER_SUCCESS)
    {
        this->set_error(out_buffer.get_error());
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

int http2_stream_manager::get_error() const noexcept
{
    return (this->_error_code);
}

const char *http2_stream_manager::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

bool http2_encode_frame(const http2_frame &frame, ft_string &out_buffer,
    int &error_code) noexcept
{
    size_t payload_length;
    unsigned char header[9];
    size_t index;
    const char *payload_data;

    out_buffer.clear();
    if (out_buffer.get_error() != ER_SUCCESS)
    {
        error_code = out_buffer.get_error();
        return (false);
    }
    payload_length = frame.payload.size();
    if (payload_length > 0xFFFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    header[0] = static_cast<unsigned char>((payload_length >> 16) & 0xFF);
    header[1] = static_cast<unsigned char>((payload_length >> 8) & 0xFF);
    header[2] = static_cast<unsigned char>(payload_length & 0xFF);
    header[3] = frame.type;
    header[4] = frame.flags;
    header[5] = static_cast<unsigned char>((frame.stream_id >> 24) & 0x7F);
    header[6] = static_cast<unsigned char>((frame.stream_id >> 16) & 0xFF);
    header[7] = static_cast<unsigned char>((frame.stream_id >> 8) & 0xFF);
    header[8] = static_cast<unsigned char>(frame.stream_id & 0xFF);
    index = 0;
    while (index < sizeof(header))
    {
        http2_append_raw_byte(out_buffer, header[index]);
        if (out_buffer.get_error() != ER_SUCCESS)
        {
            error_code = out_buffer.get_error();
            return (false);
        }
        index++;
    }
    payload_data = frame.payload.c_str();
    index = 0;
    while (index < payload_length)
    {
        http2_append_raw_byte(out_buffer,
            static_cast<unsigned char>(payload_data[index]));
        if (out_buffer.get_error() != ER_SUCCESS)
        {
            error_code = out_buffer.get_error();
            return (false);
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_decode_frame(const unsigned char *buffer, size_t buffer_size,
    size_t &offset, http2_frame &out_frame, int &error_code) noexcept
{
    size_t remaining;
    size_t payload_length;
    size_t index;

    if (!buffer)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (buffer_size < offset)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    remaining = buffer_size - offset;
    if (remaining < 9)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    payload_length = (static_cast<size_t>(buffer[offset]) << 16);
    payload_length |= (static_cast<size_t>(buffer[offset + 1]) << 8);
    payload_length |= static_cast<size_t>(buffer[offset + 2]);
    out_frame.type = buffer[offset + 3];
    out_frame.flags = buffer[offset + 4];
    out_frame.stream_id = (static_cast<uint32_t>(buffer[offset + 5] & 0x7F) << 24);
    out_frame.stream_id |= (static_cast<uint32_t>(buffer[offset + 6]) << 16);
    out_frame.stream_id |= (static_cast<uint32_t>(buffer[offset + 7]) << 8);
    out_frame.stream_id |= static_cast<uint32_t>(buffer[offset + 8]);
    if (remaining < 9 + payload_length)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    out_frame.payload.clear();
    if (out_frame.payload.get_error() != ER_SUCCESS)
    {
        error_code = out_frame.payload.get_error();
        return (false);
    }
    index = 0;
    while (index < payload_length)
    {
        http2_append_raw_byte(out_frame.payload,
            static_cast<unsigned char>(buffer[offset + 9 + index]));
        if (out_frame.payload.get_error() != ER_SUCCESS)
        {
            error_code = out_frame.payload.get_error();
            return (false);
        }
        index++;
    }
    offset += 9 + payload_length;
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_compress_headers(const ft_vector<http2_header_field> &headers,
    ft_string &out_block, int &error_code) noexcept
{
    size_t header_count;
    size_t index;

    out_block.clear();
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    header_count = headers.size();
    if (headers.get_error() != ER_SUCCESS)
    {
        error_code = headers.get_error();
        return (false);
    }
    if (header_count > 0xFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>((header_count >> 8) & 0xFF));
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>(header_count & 0xFF));
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    index = 0;
    while (index < header_count)
    {
        const http2_header_field &field = headers[index];
        size_t name_length;
        size_t value_length;
        size_t name_index;
        size_t value_index;

        name_length = field.name.size();
        if (field.name.get_error() != ER_SUCCESS)
        {
            error_code = field.name.get_error();
            return (false);
        }
        value_length = field.value.size();
        if (field.value.get_error() != ER_SUCCESS)
        {
            error_code = field.value.get_error();
            return (false);
        }
        if (name_length > 0xFFFF || value_length > 0xFFFF)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((name_length >> 8) & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(name_length & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        name_index = 0;
        while (name_index < name_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(field.name.c_str()[name_index]));
            if (out_block.get_error() != ER_SUCCESS)
            {
                error_code = out_block.get_error();
                return (false);
            }
            name_index++;
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((value_length >> 8) & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(value_length & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        value_index = 0;
        while (value_index < value_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(field.value.c_str()[value_index]));
            if (out_block.get_error() != ER_SUCCESS)
            {
                error_code = out_block.get_error();
                return (false);
            }
            value_index++;
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_decompress_headers(const ft_string &block,
    ft_vector<http2_header_field> &out_headers, int &error_code) noexcept
{
    const unsigned char *buffer;
    const unsigned char *cursor;
    const unsigned char *end;
    size_t buffer_length;
    size_t header_count;
    size_t index;

    out_headers.clear();
    buffer = reinterpret_cast<const unsigned char*>(block.c_str());
    buffer_length = block.size();
    if (block.get_error() != ER_SUCCESS)
    {
        error_code = block.get_error();
        return (false);
    }
    if (buffer_length < 2)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    header_count = (static_cast<size_t>(buffer[0]) << 8)
        | static_cast<size_t>(buffer[1]);
    cursor = buffer + 2;
    end = buffer + buffer_length;
    index = 0;
    while (index < header_count)
    {
        http2_header_field entry;
        size_t name_length;
        size_t value_length;

        if (cursor + 4 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        name_length = (static_cast<size_t>(cursor[0]) << 8)
            | static_cast<size_t>(cursor[1]);
        cursor += 2;
        if (cursor + name_length + 2 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        entry.name.assign(reinterpret_cast<const char*>(cursor), name_length);
        if (entry.name.get_error() != ER_SUCCESS)
        {
            error_code = entry.name.get_error();
            return (false);
        }
        cursor += name_length;
        value_length = (static_cast<size_t>(cursor[0]) << 8)
            | static_cast<size_t>(cursor[1]);
        cursor += 2;
        if (cursor + value_length > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        entry.value.assign(reinterpret_cast<const char*>(cursor), value_length);
        if (entry.value.get_error() != ER_SUCCESS)
        {
            error_code = entry.value.get_error();
            return (false);
        }
        cursor += value_length;
        out_headers.push_back(entry);
        if (out_headers.get_error() != ER_SUCCESS)
        {
            error_code = out_headers.get_error();
            return (false);
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_select_alpn_protocol(SSL *ssl_session, bool &selected_http2,
    int &error_code) noexcept
{
    const unsigned char *selected_protocol;
    unsigned int selected_length;
    unsigned char protocols[13];
    int result;

    selected_http2 = false;
    if (!ssl_session)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    protocols[0] = 2;
    protocols[1] = 'h';
    protocols[2] = '2';
    protocols[3] = 8;
    protocols[4] = 'h';
    protocols[5] = 't';
    protocols[6] = 't';
    protocols[7] = 'p';
    protocols[8] = '/';
    protocols[9] = '1';
    protocols[10] = '.';
    protocols[11] = '1';
    protocols[12] = '1';
    result = SSL_set_alpn_protos(ssl_session, protocols, sizeof(protocols));
    if (result != 0)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    selected_protocol = ft_nullptr;
    selected_length = 0;
    SSL_get0_alpn_selected(ssl_session, &selected_protocol, &selected_length);
    if (selected_protocol && selected_length == 2)
    {
        if (selected_protocol[0] == 'h' && selected_protocol[1] == '2')
            selected_http2 = true;
    }
    error_code = ER_SUCCESS;
    return (true);
}
