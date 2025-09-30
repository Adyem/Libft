#include <zlib.h>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"
#include "compression.hpp"
#include "compression_stream_test_hooks.hpp"

static int map_zlib_error(int zlib_status)
{
    if (zlib_status == Z_MEM_ERROR)
        return (FT_EALLOC);
    if (zlib_status == Z_BUF_ERROR)
        return (FT_EIO);
    if (zlib_status == Z_NEED_DICT)
        return (FT_EINVAL);
    if (zlib_status == Z_DATA_ERROR)
        return (FT_EINVAL);
    if (zlib_status == Z_STREAM_ERROR)
        return (FT_EINVAL);
    if (zlib_status == Z_VERSION_ERROR)
        return (FT_EINVAL);
    return (FT_EINVAL);
}

static int compress_stream_default_deflate_init(z_stream *stream, int compression_level)
{
    return (deflateInit(stream, compression_level));
}

static int compress_stream_default_deflate(z_stream *stream, int flush_mode)
{
    return (deflate(stream, flush_mode));
}

static int decompress_stream_default_inflate_init(z_stream *stream)
{
    return (inflateInit(stream));
}

static int decompress_stream_default_inflate(z_stream *stream, int flush_mode)
{
    return (inflate(stream, flush_mode));
}

static t_compress_stream_deflate_init_hook    g_compress_stream_deflate_init_hook = compress_stream_default_deflate_init;
static t_compress_stream_deflate_hook         g_compress_stream_deflate_hook = compress_stream_default_deflate;
static t_decompress_stream_inflate_init_hook  g_decompress_stream_inflate_init_hook = decompress_stream_default_inflate_init;
static t_decompress_stream_inflate_hook       g_decompress_stream_inflate_hook = decompress_stream_default_inflate;

void ft_compress_stream_set_deflate_init_hook(t_compress_stream_deflate_init_hook hook)
{
    if (hook)
        g_compress_stream_deflate_init_hook = hook;
    else
        g_compress_stream_deflate_init_hook = compress_stream_default_deflate_init;
    return ;
}

void ft_compress_stream_set_deflate_hook(t_compress_stream_deflate_hook hook)
{
    if (hook)
        g_compress_stream_deflate_hook = hook;
    else
        g_compress_stream_deflate_hook = compress_stream_default_deflate;
    return ;
}

void ft_decompress_stream_set_inflate_init_hook(t_decompress_stream_inflate_init_hook hook)
{
    if (hook)
        g_decompress_stream_inflate_init_hook = hook;
    else
        g_decompress_stream_inflate_init_hook = decompress_stream_default_inflate_init;
    return ;
}

void ft_decompress_stream_set_inflate_hook(t_decompress_stream_inflate_hook hook)
{
    if (hook)
        g_decompress_stream_inflate_hook = hook;
    else
        g_decompress_stream_inflate_hook = decompress_stream_default_inflate;
    return ;
}

int ft_compress_stream(int input_fd, int output_fd)
{
    z_stream        stream;
    unsigned char   input_buffer[4096];
    unsigned char   output_buffer[4096];
    int             flush_mode;
    int             deflate_status;
    ssize_t         read_bytes;

    if (input_fd < 0 || output_fd < 0)
    {
        ft_errno = FT_EINVAL;
        return (1);
    }
    ft_bzero(&stream, sizeof(stream));
    deflate_status = g_compress_stream_deflate_init_hook(&stream, Z_BEST_COMPRESSION);
    if (deflate_status != Z_OK)
    {
        ft_errno = map_zlib_error(deflate_status);
        return (1);
    }
    flush_mode = Z_NO_FLUSH;
    while (flush_mode != Z_FINISH)
    {
        read_bytes = su_read(input_fd, input_buffer, sizeof(input_buffer));
        if (read_bytes < 0)
        {
            deflateEnd(&stream);
            ft_errno = FT_EIO;
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
            deflate_status = g_compress_stream_deflate_hook(&stream, flush_mode);
            if (deflate_status == Z_STREAM_ERROR || deflate_status == Z_BUF_ERROR)
            {
                deflateEnd(&stream);
                ft_errno = map_zlib_error(deflate_status);
                return (1);
            }
            std::size_t produced_bytes = sizeof(output_buffer) - stream.avail_out;
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                deflateEnd(&stream);
                ft_errno = FT_EIO;
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    deflateEnd(&stream);
    ft_errno = ER_SUCCESS;
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
    {
        ft_errno = FT_EINVAL;
        return (1);
    }
    ft_bzero(&stream, sizeof(stream));
    inflate_status = g_decompress_stream_inflate_init_hook(&stream);
    if (inflate_status != Z_OK)
    {
        ft_errno = map_zlib_error(inflate_status);
        return (1);
    }
    flush_mode = Z_NO_FLUSH;
    while (flush_mode != Z_FINISH)
    {
        read_bytes = su_read(input_fd, input_buffer, sizeof(input_buffer));
        if (read_bytes < 0)
        {
            inflateEnd(&stream);
            ft_errno = FT_EIO;
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
            inflate_status = g_decompress_stream_inflate_hook(&stream, flush_mode);
            if (inflate_status == Z_NEED_DICT || inflate_status == Z_DATA_ERROR || inflate_status == Z_MEM_ERROR)
            {
                inflateEnd(&stream);
                ft_errno = map_zlib_error(inflate_status);
                return (1);
            }
            std::size_t produced_bytes = sizeof(output_buffer) - stream.avail_out;
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                inflateEnd(&stream);
                ft_errno = FT_EIO;
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    inflateEnd(&stream);
    ft_errno = ER_SUCCESS;
    return (0);
}
