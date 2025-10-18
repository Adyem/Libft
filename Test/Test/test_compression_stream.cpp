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

static int compression_stream_buf_error_deflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    return (Z_BUF_ERROR);
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

static int compression_stream_buf_error_inflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    return (Z_BUF_ERROR);
}

static int g_inflate_stream_end_extra_calls = 0;

static int g_compress_flush_first_non_finish = -1;
static int g_compress_flush_finish_count = 0;

static int compression_stream_capture_flush_deflate(z_stream *stream, int flush_mode)
{
    if (flush_mode != Z_FINISH && g_compress_flush_first_non_finish == -1)
        g_compress_flush_first_non_finish = flush_mode;
    if (flush_mode == Z_FINISH)
        g_compress_flush_finish_count += 1;
    return (deflate(stream, flush_mode));
}

static int g_decompress_last_flush_mode = -1;

static int compression_stream_capture_flush_inflate(z_stream *stream, int flush_mode)
{
    g_decompress_last_flush_mode = flush_mode;
    return (inflate(stream, flush_mode));
}

static int g_compress_first_hook_calls = 0;
static int g_compress_second_hook_calls = 0;

static int compression_stream_counting_deflate(z_stream *stream, int flush_mode)
{
    g_compress_first_hook_calls += 1;
    return (deflate(stream, flush_mode));
}

static int compression_stream_failing_replacement_deflate(z_stream *stream, int flush_mode)
{
    (void)stream;
    (void)flush_mode;
    g_compress_second_hook_calls += 1;
    return (Z_STREAM_ERROR);
}

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

static std::size_t  compression_stream_collect_compressed_size(const char *payload,
        std::size_t payload_size,
        t_compress_stream_options *options)
{
    int             input_pipe[2];
    int             output_pipe[2];
    ssize_t         written_bytes;
    int             result;
    std::size_t     total_size;
    unsigned char   buffer[1024];
    ssize_t         read_bytes;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1], payload, payload_size);
    FT_ASSERT_EQ(static_cast<ssize_t>(payload_size), written_bytes);
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream_with_options(input_pipe[0], output_pipe[1], options);
    close(input_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    total_size = 0;
    while (1)
    {
        read_bytes = su_read(output_pipe[0], buffer, sizeof(buffer));
        FT_ASSERT(read_bytes >= 0);
        if (read_bytes == 0)
            break ;
        total_size += static_cast<std::size_t>(read_bytes);
    }
    close(output_pipe[0]);
    return (total_size);
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

FT_TEST(test_ft_compress_stream_with_options_rejects_zero_buffer, "ft_compress_stream_with_options rejects zero-sized buffers")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    t_compress_stream_options   options;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(0));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(16));
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_with_options_supports_custom_buffers, "ft_compress_stream_with_options accepts custom buffer sizes")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    t_compress_stream_options   options;
    const char                  *payload;
    ssize_t                     written_bytes;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(8));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(12));
    payload = "custom buffer payload";
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written_bytes);
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_compress_stream_speed_preset_sets_expected_values, "speed preset configures throughput tuned defaults")
{
    t_compress_stream_options   options;

    ft_compress_stream_apply_speed_preset(&options);
    FT_ASSERT_EQ(static_cast<std::size_t>(16384), options.get_input_buffer_size());
    FT_ASSERT_EQ(static_cast<std::size_t>(16384), options.get_output_buffer_size());
    FT_ASSERT_EQ(Z_BEST_SPEED, options.get_compression_level());
    FT_ASSERT(options.get_window_bits() >= 8);
    FT_ASSERT(options.get_window_bits() <= MAX_WBITS);
    FT_ASSERT(options.get_memory_level() >= 1);
    FT_ASSERT(options.get_memory_level() <= 9);
    FT_ASSERT_EQ(Z_DEFAULT_STRATEGY, options.get_strategy());
    return (1);
}

FT_TEST(test_ft_compress_stream_ratio_preset_sets_expected_values, "ratio preset configures aggressive compression defaults")
{
    t_compress_stream_options   speed_options;
    t_compress_stream_options   ratio_options;

    ft_compress_stream_apply_speed_preset(&speed_options);
    ft_compress_stream_apply_ratio_preset(&ratio_options);
    FT_ASSERT_EQ(static_cast<std::size_t>(32768), ratio_options.get_input_buffer_size());
    FT_ASSERT_EQ(static_cast<std::size_t>(32768), ratio_options.get_output_buffer_size());
    FT_ASSERT_EQ(Z_BEST_COMPRESSION, ratio_options.get_compression_level());
    FT_ASSERT_EQ(MAX_WBITS, ratio_options.get_window_bits());
    FT_ASSERT(ratio_options.get_window_bits() >= speed_options.get_window_bits());
    FT_ASSERT(ratio_options.get_memory_level() >= speed_options.get_memory_level());
    FT_ASSERT(ratio_options.get_memory_level() <= 9);
    FT_ASSERT_EQ(Z_DEFAULT_STRATEGY, ratio_options.get_strategy());
    return (1);
}

FT_TEST(test_ft_compress_stream_ratio_preset_outperforms_speed_preset, "ratio preset yields smaller payloads than speed preset")
{
    ft_string                   payload;
    t_compress_stream_options   speed_options;
    t_compress_stream_options   ratio_options;
    std::size_t                 speed_size;
    std::size_t                 ratio_size;

    payload.assign(32768, 'A');
    FT_ASSERT_EQ(ER_SUCCESS, payload.get_error());
    ft_compress_stream_apply_speed_preset(&speed_options);
    ft_compress_stream_apply_ratio_preset(&ratio_options);
    speed_size = compression_stream_collect_compressed_size(payload.data(), payload.size(), &speed_options);
    ratio_size = compression_stream_collect_compressed_size(payload.data(), payload.size(), &ratio_options);
    FT_ASSERT(speed_size >= ratio_size);
    return (1);
}

FT_TEST(test_ft_compress_stream_rejects_invalid_window_bits, "ft_compress_stream rejects out-of-range window sizes")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    t_compress_stream_options   options;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(16));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(16));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_window_bits(16));
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
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

FT_TEST(test_ft_compress_stream_reports_buffer_exhaustion, "ft_compress_stream reports buffer exhaustion distinctly")
{
    int input_pipe[2];
    int output_pipe[2];
    const char *payload;
    ssize_t written;
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    payload = "buffer exhaustion";
    written = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written);
    close(input_pipe[1]);
    ft_compress_stream_set_deflate_hook(compression_stream_buf_error_deflate);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    ft_compress_stream_set_deflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
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

FT_TEST(test_ft_decompress_stream_with_options_rejects_zero_buffer, "ft_decompress_stream_with_options rejects zero-sized buffers")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    t_compress_stream_options   options;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(4));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(0));
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_with_options_supports_custom_buffers, "ft_decompress_stream_with_options accepts custom buffer sizes")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    const unsigned char         *payload;
    unsigned char               *compressed_buffer;
    std::size_t                 compressed_size;
    ssize_t                     written_bytes;
    t_compress_stream_options   options;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(10));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(7));
    payload = reinterpret_cast<const unsigned char *>("custom decompress payload");
    compressed_size = 0;
    compressed_buffer = ft_compress(payload, ft_strlen_size_t("custom decompress payload"), &compressed_size);
    FT_ASSERT(compressed_buffer != ft_nullptr);
    FT_ASSERT(compressed_size > sizeof(uint32_t));
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1],
            compressed_buffer + sizeof(uint32_t),
            compressed_size - sizeof(uint32_t));
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size - sizeof(uint32_t)), written_bytes);
    close(input_pipe[1]);
    cma_free(compressed_buffer);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
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

FT_TEST(test_ft_decompress_stream_reports_buffer_exhaustion, "ft_decompress_stream reports buffer exhaustion distinctly")
{
    int input_pipe[2];
    int output_pipe[2];
    int result;

    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    close(input_pipe[1]);
    ft_decompress_stream_set_inflate_hook(compression_stream_buf_error_inflate);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
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

FT_TEST(test_ft_compress_stream_uses_finish_flush_after_partial_read, "ft_compress_stream uses Z_FINISH after the final read")
{
    int                         input_pipe[2];
    int                         output_pipe[2];
    t_compress_stream_options   options;
    const char                  *payload;
    ssize_t                     written_bytes;
    int                         result;

    FT_ASSERT_EQ(ER_SUCCESS, options.set_input_buffer_size(4));
    FT_ASSERT_EQ(ER_SUCCESS, options.set_output_buffer_size(64));
    payload = "partial-flush";
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written_bytes);
    close(input_pipe[1]);
    g_compress_flush_first_non_finish = -1;
    g_compress_flush_finish_count = 0;
    ft_compress_stream_set_deflate_hook(compression_stream_capture_flush_deflate);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream_with_options(input_pipe[0], output_pipe[1], &options);
    ft_compress_stream_set_deflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(Z_NO_FLUSH, g_compress_flush_first_non_finish);
    FT_ASSERT(g_compress_flush_finish_count > 0);
    return (1);
}

FT_TEST(test_ft_decompress_stream_uses_finish_flush_at_end, "ft_decompress_stream switches to Z_FINISH at EOF")
{
    const char                  *payload;
    unsigned char               compressed_buffer[256];
    uLongf                      compressed_size;
    int                         zlib_status;
    int                         input_pipe[2];
    int                         output_pipe[2];
    int                         result;

    payload = "stream flush verification";
    compressed_size = sizeof(compressed_buffer);
    zlib_status = compress2(compressed_buffer, &compressed_size,
            reinterpret_cast<const Bytef *>(payload), static_cast<uLong>(ft_strlen_size_t(payload)), Z_BEST_COMPRESSION);
    FT_ASSERT_EQ(Z_OK, zlib_status);
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size),
        su_write(input_pipe[1], compressed_buffer, compressed_size));
    close(input_pipe[1]);
    g_decompress_last_flush_mode = -1;
    ft_decompress_stream_set_inflate_hook(compression_stream_capture_flush_inflate);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    ft_decompress_stream_set_inflate_hook(ft_nullptr);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(Z_FINISH, g_decompress_last_flush_mode);
    return (1);
}

FT_TEST(test_compress_stream_hooks_replace_incrementally, "ft_compress_stream accepts incremental hook replacement")
{
    int         input_pipe[2];
    int         output_pipe[2];
    const char  *payload;
    ssize_t     written_bytes;
    int         result;

    payload = "hook rotation";
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1], payload, ft_strlen_size_t(payload));
    FT_ASSERT_EQ(static_cast<ssize_t>(ft_strlen_size_t(payload)), written_bytes);
    close(input_pipe[1]);
    g_compress_first_hook_calls = 0;
    g_compress_second_hook_calls = 0;
    ft_compress_stream_set_deflate_hook(compression_stream_counting_deflate);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(g_compress_first_hook_calls > 0);
    ft_compress_stream_set_deflate_hook(compression_stream_failing_replacement_deflate);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT(g_compress_second_hook_calls > 0);
    ft_compress_stream_set_deflate_hook(ft_nullptr);
    ft_errno = ER_SUCCESS;
    result = ft_compress_stream(input_pipe[0], output_pipe[1]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_decompress_stream_reports_truncated_input, "ft_decompress_stream fails on truncated inputs")
{
    const char        *payload;
    unsigned char     compressed_buffer[256];
    uLongf            compressed_size;
    int               zlib_status;
    int               input_pipe[2];
    int               output_pipe[2];
    ssize_t           written_bytes;
    int               result;

    payload = "truncate";
    compressed_size = sizeof(compressed_buffer);
    zlib_status = compress2(compressed_buffer, &compressed_size,
            reinterpret_cast<const Bytef *>(payload), static_cast<uLong>(ft_strlen_size_t(payload)), Z_BEST_COMPRESSION);
    FT_ASSERT_EQ(Z_OK, zlib_status);
    FT_ASSERT(compressed_size > 1);
    FT_ASSERT_EQ(0, pipe(input_pipe));
    FT_ASSERT_EQ(0, pipe(output_pipe));
    written_bytes = su_write(input_pipe[1], compressed_buffer, compressed_size - 1);
    FT_ASSERT_EQ(static_cast<ssize_t>(compressed_size - 1), written_bytes);
    close(input_pipe[1]);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_stream(input_pipe[0], output_pipe[1]);
    close(input_pipe[0]);
    close(output_pipe[0]);
    close(output_pipe[1]);
    FT_ASSERT_EQ(1, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
