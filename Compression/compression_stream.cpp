#include <zlib.h>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "compression.hpp"

int ft_compress_stream(int input_fd, int output_fd)
{
    z_stream        stream;
    unsigned char   input_buffer[4096];
    unsigned char   output_buffer[4096];
    int             flush_mode;
    int             deflate_status;
    ssize_t         read_bytes;

    if (input_fd < 0 || output_fd < 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK)
        return (1);
    flush_mode = Z_NO_FLUSH;
    while (flush_mode != Z_FINISH)
    {
        read_bytes = su_read(input_fd, input_buffer, sizeof(input_buffer));
        if (read_bytes < 0)
        {
            deflateEnd(&stream);
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (static_cast<std::size_t>(read_bytes) < sizeof(input_buffer))
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = sizeof(output_buffer);
            deflate_status = deflate(&stream, flush_mode);
            if (deflate_status == Z_STREAM_ERROR)
            {
                deflateEnd(&stream);
                return (1);
            }
            std::size_t produced_bytes = sizeof(output_buffer) - stream.avail_out;
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                deflateEnd(&stream);
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    deflateEnd(&stream);
    return (0);
}

int ft_decompress_stream(int input_fd, int output_fd)
{
    z_stream        stream;
    unsigned char   input_buffer[4096];
    unsigned char   output_buffer[4096];
    int             inflate_status;
    ssize_t         read_bytes;
    int             flush_mode;

    if (input_fd < 0 || output_fd < 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    if (inflateInit(&stream) != Z_OK)
        return (1);
    flush_mode = Z_NO_FLUSH;
    while (flush_mode != Z_FINISH)
    {
        read_bytes = su_read(input_fd, input_buffer, sizeof(input_buffer));
        if (read_bytes < 0)
        {
            inflateEnd(&stream);
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (read_bytes == 0)
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = sizeof(output_buffer);
            inflate_status = inflate(&stream, flush_mode);
            if (inflate_status == Z_NEED_DICT || inflate_status == Z_DATA_ERROR || inflate_status == Z_MEM_ERROR)
            {
                inflateEnd(&stream);
                return (1);
            }
            std::size_t produced_bytes = sizeof(output_buffer) - stream.avail_out;
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                inflateEnd(&stream);
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    inflateEnd(&stream);
    return (0);
}
