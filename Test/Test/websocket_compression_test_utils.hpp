#ifndef WEBSOCKET_COMPRESSION_TEST_UTILS_HPP
#define WEBSOCKET_COMPRESSION_TEST_UTILS_HPP

#include "websocket_test_utils.hpp"
#include "../../Template/vector.hpp"
#include <zlib.h>

static bool websocket_permessage_deflate_compress(const ft_string &message,
    ft_vector<unsigned char> &compressed)
{
    z_stream stream;
    int zlib_result;

    compressed.clear();
    ft_memset(&stream, 0, sizeof(stream));
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(message.c_str()));
    stream.avail_in = static_cast<uInt>(message.size());
    zlib_result = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (zlib_result != Z_OK)
        return (false);
    while (true)
    {
        unsigned char chunk[256];
        std::size_t produced_bytes;
        std::size_t index_value;

        stream.next_out = chunk;
        stream.avail_out = sizeof(chunk);
        zlib_result = deflate(&stream, Z_SYNC_FLUSH);
        produced_bytes = sizeof(chunk) - stream.avail_out;
        index_value = 0;
        while (index_value < produced_bytes)
        {
            compressed.push_back(chunk[index_value]);
            if (compressed.get_error() != ER_SUCCESS)
            {
                deflateEnd(&stream);
                return (false);
            }
            index_value++;
        }
        if (zlib_result == Z_STREAM_END)
            break ;
        if (stream.avail_in == 0 && stream.avail_out != 0)
            break ;
        if (zlib_result != Z_OK)
        {
            deflateEnd(&stream);
            return (false);
        }
    }
    deflateEnd(&stream);
    if (compressed.size() < 4)
        return (false);
    std::size_t trim_index;

    trim_index = 0;
    while (trim_index < 4)
    {
        compressed.pop_back();
        if (compressed.get_error() != ER_SUCCESS)
            return (false);
        trim_index++;
    }
    return (true);
}

static bool websocket_permessage_deflate_decompress(const unsigned char *payload,
    std::size_t payload_length, ft_string &message)
{
    ft_vector<unsigned char> input_buffer;
    ft_vector<unsigned char> output_buffer;
    unsigned char tail_bytes[4];
    z_stream stream;
    int zlib_result;

    if (payload == ft_nullptr && payload_length != 0)
        return (false);
    tail_bytes[0] = 0x00;
    tail_bytes[1] = 0x00;
    tail_bytes[2] = 0xFF;
    tail_bytes[3] = 0xFF;
    input_buffer.reserve(payload_length + 4);
    if (input_buffer.get_error() != ER_SUCCESS)
        return (false);
    std::size_t index_value;

    index_value = 0;
    while (index_value < payload_length)
    {
        input_buffer.push_back(payload[index_value]);
        if (input_buffer.get_error() != ER_SUCCESS)
            return (false);
        index_value++;
    }
    index_value = 0;
    while (index_value < 4)
    {
        input_buffer.push_back(tail_bytes[index_value]);
        if (input_buffer.get_error() != ER_SUCCESS)
            return (false);
        index_value++;
    }
    ft_memset(&stream, 0, sizeof(stream));
    stream.next_in = const_cast<Bytef *>(input_buffer.begin());
    stream.avail_in = static_cast<uInt>(input_buffer.size());
    zlib_result = inflateInit2(&stream, -15);
    if (zlib_result != Z_OK)
        return (false);
    while (true)
    {
        unsigned char chunk[256];
        std::size_t produced_bytes;

        stream.next_out = chunk;
        stream.avail_out = sizeof(chunk);
        zlib_result = inflate(&stream, Z_SYNC_FLUSH);
        produced_bytes = sizeof(chunk) - stream.avail_out;
        index_value = 0;
        while (index_value < produced_bytes)
        {
            output_buffer.push_back(chunk[index_value]);
            if (output_buffer.get_error() != ER_SUCCESS)
            {
                inflateEnd(&stream);
                return (false);
            }
            index_value++;
        }
        if (zlib_result == Z_STREAM_END)
            break ;
        if (zlib_result == Z_BUF_ERROR && stream.avail_in == 0)
            break ;
        if (zlib_result != Z_OK && zlib_result != Z_BUF_ERROR)
        {
            inflateEnd(&stream);
            return (false);
        }
    }
    inflateEnd(&stream);
    message.assign(reinterpret_cast<const char *>(output_buffer.begin()), output_buffer.size());
    if (message.get_error() != ER_SUCCESS)
        return (false);
    return (true);
}

#endif
