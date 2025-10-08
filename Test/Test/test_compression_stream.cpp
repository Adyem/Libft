#include "../../Compression/compression.hpp"
#include "../../Compression/compression_stream_test_hooks.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <unistd.h>
#include <cstdint>

static int compression_stream_fail_deflate_init(z_stream *stream, int compression_level)
{
    (void)stream;
    (void)compression_level;
    return (Z_MEM_ERROR);
}

static int compression_stream_fail_deflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    return (Z_STREAM_ERROR);
}

static int compression_stream_fail_inflate_init(z_stream *stream)
{
    (void)stream;
    return (Z_MEM_ERROR);
}

static int compression_stream_fail_inflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    return (Z_DATA_ERROR);
}

static int compression_stream_stream_error_inflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    return (Z_STREAM_ERROR);
}

static int g_inflate_stream_end_extra_calls = 0;

static int compression_stream_trailing_bytes_inflate(z_stream *stream, int flush_mode)
{
    (void)flush_mode;
    if (g_inflate_stream_end_extra_calls == 0)
    {
        g_inflate_stream_end_extra_calls = 1;
        if (stream->avail_out > 0)
        {
            stream->next_out[0] = 'X';
            stream->avail_out -= 1;
        }
        if (stream->avail_in > 0)
            stream->avail_in = 1;
        return (Z_STREAM_END);
    }
    return (Z_STREAM_END);
}

FT_TEST(test_ft_compress_stream_rejects_invalid_descriptors, "ft_compress_stream rejects invalid descriptors")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(-1, -1);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_reports_deflate_init_failure, "ft_compress_stream reports deflateInit failures")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    ft_compress_stream_set_deflate_init_hook(compression_stream_fail_deflate_init);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    ft_compress_stream_set_deflate_init_hook(ft_nullptr);
    close(input_pipe[0]);
    close(input_pipe[1]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_reports_read_failure, "ft_compress_stream reports su_read failures")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[0]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[1], output_pipe[1]);
    close(input_pipe[1]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_reports_deflate_failure, "ft_compress_stream reports deflate failures")
{
    int input_pipe[2];
    int output_pipe[2];
    const char *payload;
    ssize_t written;
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    payload = "stream failure";
    written = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written);
    close(input_pipe[1]);
    ft_compress_stream_set_deflate_hook(compression_stream_fail_deflate);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    ft_compress_stream_set_deflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_reports_write_failure, "ft_compress_stream reports su_write failures")
{
    int input_pipe[2];
    int output_pipe[2];
    const char *payload;
    ssize_t written;
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    payload = "stream write";
    written = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written);
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[0]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_success_sets_errno_success, "ft_compress_stream success sets ft_errno")
{
    int input_pipe[2];
    int output_pipe[2];
    const char *payload;
    ssize_t written;
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    payload = "compress success";
    written = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written);
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_rejects_invalid_descriptors, "ft_decompress_stream rejects invalid descriptors")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(-1, -1);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_inflate_init_failure, "ft_decompress_stream reports inflateInit failures")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    ft_decompress_stream_set_inflate_init_hook(compression_stream_fail_inflate_init);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_init_hook(ft_nullptr);
    close(input_pipe[0]);
    close(input_pipe[1]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_read_failure, "ft_decompress_stream reports su_read failures")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[0]);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[1], output_pipe[1]);
    close(input_pipe[1]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_inflate_failure, "ft_decompress_stream reports inflate failures")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[1]);
    ft_decompress_stream_set_inflate_hook(compression_stream_fail_inflate);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_stream_errors, "ft_decompress_stream reports stream errors")
{
    int input_pipe[2];
    int output_pipe[2];
    const unsigned char *payload;
    unsigned char *compressed_buffer;
    std::size_t compressed_size;
    ssize_t payload_written;
    int result;

    payload = reinterpret_cast<const unsigned char *>("stream error");
    compressed_size = 0;
    compressed_buffer = ft_compress(payload, ft_strlen_size_t("stream error"), &compressed_size);
    FT_ASSERT(compressed_buffer != ft_nullptr);
    FT_ASSERT(compressed_size > 0);
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    FT_ASSERT(compressed_size > sizeof(uint32_t));
    payload_written = su_write(input_pipe[1],
            compressed_buffer + sizeof(uint32_t),
            compressed_size - sizeof(uint32_t));
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size - sizeof(uint32_t)),
        payload_written);
    close(input_pipe[1]);
    cma_free(compressed_buffer);
    ft_decompress_stream_set_inflate_hook(compression_stream_stream_error_inflate);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_write_failure, "ft_decompress_stream reports su_write failures")
{
    int input_pipe[2];
    int output_pipe[2];
    const unsigned char *payload;
    unsigned char *compressed_buffer;
    std::size_t compressed_size;
    ssize_t payload_written;
    int result;

    payload = reinterpret_cast<const unsigned char *>("decompress write");
    compressed_size = 0;
    compressed_buffer = ft_compress(payload, ft_strlen_size_t("decompress write"), &compressed_size);
    FT_ASSERT(compressed_buffer != ft_nullptr);
    FT_ASSERT(compressed_size > 0);
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    FT_ASSERT(compressed_size > sizeof(uint32_t));
    payload_written = su_write(input_pipe[1],
            compressed_buffer + sizeof(uint32_t),
            compressed_size - sizeof(uint32_t));
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size - sizeof(uint32_t)),
        payload_written);
    close(input_pipe[1]);
    cma_free(compressed_buffer);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[0]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_success_sets_errno_success, "ft_decompress_stream success sets ft_errno")
{
    int input_pipe[2];
    int output_pipe[2];
    const unsigned char *payload;
    unsigned char *compressed_buffer;
    std::size_t compressed_size;
    ssize_t payload_written;
    int result;

    payload = reinterpret_cast<const unsigned char *>("decompress success");
    compressed_size = 0;
    compressed_buffer = ft_compress(payload, ft_strlen_size_t("decompress success"), &compressed_size);
    FT_ASSERT(compressed_buffer != ft_nullptr);
    FT_ASSERT(compressed_size > 0);
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    FT_ASSERT(compressed_size > sizeof(uint32_t));
    payload_written = su_write(input_pipe[1],
            compressed_buffer + sizeof(uint32_t),
            compressed_size - sizeof(uint32_t));
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size - sizeof(uint32_t)),
        payload_written);
    close(input_pipe[1]);
    cma_free(compressed_buffer);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_rejects_trailing_bytes, "ft_decompress_stream rejects trailing input")
{
    int             input_pipe[2];
    int             output_pipe[2];
    unsigned char   payload[4];
    ssize_t         payload_written;
    int             result;

    payload[0] = 0x01;
    payload[1] = 0x02;
    payload[2] = 0x03;
    payload[3] = 0x04;
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    payload_written = su_write(input_pipe[1], payload, sizeof(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(sizeof(payload)), payload_written);
    close(input_pipe[1]);
    g_inflate_stream_end_extra_calls = 0;
    ft_decompress_stream_set_inflate_hook(compression_stream_trailing_bytes_inflate);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
