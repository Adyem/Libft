#include <zlib.h>
#include <limits>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"
#include "compression.hpp"
#include "compression_stream_test_hooks.hpp"

static const std::size_t   g_compress_stream_default_buffer_size = 4096;

static int  map_zlib_error(int zlib_status)
{
    if (zlib_status == Z_MEM_ERROR)
        return (FT_ERR_NO_MEMORY);
    if (zlib_status == Z_BUF_ERROR)
        return (FT_ERR_FULL);
    if (zlib_status == Z_NEED_DICT)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_DATA_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_STREAM_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_VERSION_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_INVALID_ARGUMENT);
}

static void compression_stream_release_buffers(unsigned char *input_buffer, unsigned char *output_buffer)
{
    int saved_errno;

    saved_errno = ft_errno;
    if (input_buffer)
        cma_free(input_buffer);
    if (output_buffer)
        cma_free(output_buffer);
    ft_errno = saved_errno;
    return ;
}

static int  compression_stream_allocate_buffers(const t_compress_stream_options *options,
        unsigned char **input_buffer,
        std::size_t *input_buffer_size,
        unsigned char **output_buffer,
        std::size_t *output_buffer_size)
{
    std::size_t    resolved_input_size;
    std::size_t    resolved_output_size;

    if (input_buffer == ft_nullptr || input_buffer_size == ft_nullptr
        || output_buffer == ft_nullptr || output_buffer_size == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (options)
    {
        resolved_input_size = options->input_buffer_size;
        resolved_output_size = options->output_buffer_size;
        if (resolved_input_size == 0 || resolved_output_size == 0)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    else
    {
        resolved_input_size = g_compress_stream_default_buffer_size;
        resolved_output_size = g_compress_stream_default_buffer_size;
    }
    if (resolved_input_size > std::numeric_limits<unsigned int>::max()
        || resolved_output_size > std::numeric_limits<unsigned int>::max())
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    *input_buffer = static_cast<unsigned char *>(cma_malloc(resolved_input_size));
    if (!*input_buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (1);
    }
    *output_buffer = static_cast<unsigned char *>(cma_malloc(resolved_output_size));
    if (!*output_buffer)
    {
        cma_free(*input_buffer);
        *input_buffer = ft_nullptr;
        ft_errno = FT_ERR_NO_MEMORY;
        return (1);
    }
    *input_buffer_size = resolved_input_size;
    *output_buffer_size = resolved_output_size;
    return (0);
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

int ft_compress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options)
{
    z_stream        stream;
    unsigned char   *input_buffer;
    unsigned char   *output_buffer;
    std::size_t     input_buffer_size;
    std::size_t     output_buffer_size;
    int             flush_mode;
    int             deflate_status;
    ssize_t         read_bytes;

    if (input_fd < 0 || output_fd < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    input_buffer = ft_nullptr;
    output_buffer = ft_nullptr;
    input_buffer_size = 0;
    output_buffer_size = 0;
    if (compression_stream_allocate_buffers(options, &input_buffer, &input_buffer_size,
            &output_buffer, &output_buffer_size) != 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    deflate_status = g_compress_stream_deflate_init_hook(&stream, Z_BEST_COMPRESSION);
    if (deflate_status != Z_OK)
    {
        int error_code;

        error_code = map_zlib_error(deflate_status);
        compression_stream_release_buffers(input_buffer, output_buffer);
        ft_errno = error_code;
        return (1);
    }
    flush_mode = Z_NO_FLUSH;
    while (flush_mode != Z_FINISH)
    {
        read_bytes = su_read(input_fd, input_buffer, input_buffer_size);
        if (read_bytes < 0)
        {
            deflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_IO;
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (static_cast<std::size_t>(read_bytes) < input_buffer_size)
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = static_cast<unsigned int>(output_buffer_size);
            deflate_status = g_compress_stream_deflate_hook(&stream, flush_mode);
            if (deflate_status == Z_STREAM_ERROR || deflate_status == Z_BUF_ERROR)
            {
                int error_code;

                error_code = map_zlib_error(deflate_status);
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = error_code;
                return (1);
            }
            std::size_t produced_bytes = output_buffer_size - static_cast<std::size_t>(stream.avail_out);
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_IO;
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    deflateEnd(&stream);
    compression_stream_release_buffers(input_buffer, output_buffer);
    ft_errno = ER_SUCCESS;
    return (0);
}

int ft_decompress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options)
{
    z_stream        stream;
    unsigned char   *input_buffer;
    unsigned char   *output_buffer;
    std::size_t     input_buffer_size;
    std::size_t     output_buffer_size;
    int             inflate_status;
    ssize_t         read_bytes;
    int             flush_mode;
    int             stream_finished;
    int             read_any_input;

    if (input_fd < 0 || output_fd < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    input_buffer = ft_nullptr;
    output_buffer = ft_nullptr;
    input_buffer_size = 0;
    output_buffer_size = 0;
    if (compression_stream_allocate_buffers(options, &input_buffer, &input_buffer_size,
            &output_buffer, &output_buffer_size) != 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    inflate_status = g_decompress_stream_inflate_init_hook(&stream);
    if (inflate_status != Z_OK)
    {
        compression_stream_release_buffers(input_buffer, output_buffer);
        ft_errno = map_zlib_error(inflate_status);
        return (1);
    }
    stream_finished = 0;
    read_any_input = 0;
    flush_mode = Z_NO_FLUSH;
    while (1)
    {
        read_bytes = su_read(input_fd, input_buffer, input_buffer_size);
        if (read_bytes < 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_IO;
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (read_bytes > 0)
            read_any_input = 1;
        if (read_bytes == 0)
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = static_cast<unsigned int>(output_buffer_size);
            inflate_status = g_decompress_stream_inflate_hook(&stream, flush_mode);
            if (inflate_status == Z_STREAM_END)
                stream_finished = 1;
            if (inflate_status == Z_BUF_ERROR && stream_finished == 0)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                if (read_any_input == 0)
                    ft_errno = FT_ERR_FULL;
                else
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (1);
            }
            if (inflate_status == Z_NEED_DICT
                || inflate_status == Z_DATA_ERROR
                || inflate_status == Z_MEM_ERROR
                || inflate_status == Z_STREAM_ERROR
                || inflate_status == Z_VERSION_ERROR)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = map_zlib_error(inflate_status);
                return (1);
            }
            std::size_t produced_bytes = output_buffer_size - static_cast<std::size_t>(stream.avail_out);
            if (produced_bytes != 0
                && su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_IO;
                return (1);
            }
        }
        while (stream.avail_out == 0 && stream_finished == 0 && stream.avail_in != 0);
        if (stream_finished != 0 && stream.avail_in != 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
        if (stream_finished != 0)
        {
            if (flush_mode == Z_FINISH)
                break ;
            continue ;
        }
        if (flush_mode == Z_FINISH)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    inflateEnd(&stream);
    compression_stream_release_buffers(input_buffer, output_buffer);
    ft_errno = ER_SUCCESS;
    return (0);
}

int ft_compress_stream(int input_fd, int output_fd)
{
    return (ft_compress_stream_with_options(input_fd, output_fd, ft_nullptr));
}

int ft_decompress_stream(int input_fd, int output_fd)
{
    return (ft_decompress_stream_with_options(input_fd, output_fd, ft_nullptr));
}
