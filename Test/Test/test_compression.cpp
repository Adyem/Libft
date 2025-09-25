#include "../../Compression/compression.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../System_utils/system_utils.hpp"
#include <unistd.h>

FT_TEST(test_compress_rejects_oversize_input, "compress rejects oversize input")
{
    unsigned char   input_byte;
    unsigned char   *compressed_buffer;
    std::size_t     oversize_length;
    std::size_t     compressed_length;

    if (compression_max_size == SIZE_MAX)
        return (1);
    input_byte = 0;
    oversize_length = compression_max_size;
    oversize_length++;
    compressed_length = 42;
    ft_errno = ER_SUCCESS;
    compressed_buffer = compress_buffer(&input_byte, oversize_length, &compressed_length);
    if (compressed_buffer)
    {
        cma_free(compressed_buffer);
        return (0);
    }
    if (compressed_length != 0)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_decompress_rejects_oversize_payload, "decompress rejects oversize payload")
{
    unsigned char   header[sizeof(uint32_t)];
    unsigned char   *decompressed_buffer;
    std::size_t     fake_input_size;
    std::size_t     decompressed_length;

    if (compression_max_size > SIZE_MAX - sizeof(uint32_t) - 1)
        return (1);
    ft_memset(header, 0, sizeof(uint32_t));
    fake_input_size = compression_max_size + sizeof(uint32_t) + 1;
    decompressed_length = 99;
    ft_errno = ER_SUCCESS;
    decompressed_buffer = decompress_buffer(header, fake_input_size, &decompressed_length);
    if (decompressed_buffer)
    {
        cma_free(decompressed_buffer);
        return (0);
    }
    if (decompressed_length != 0)
        return (0);
    if (ft_errno != FT_EINVAL)
        return (0);
    return (1);
}

FT_TEST(test_ft_compress_round_trip, "ft_compress round trip")
{
    const char      *input_string;
    unsigned char   *compressed_buffer;
    unsigned char   *decompressed_buffer;
    std::size_t     input_length;
    std::size_t     compressed_length;
    std::size_t     decompressed_length;
    int             comparison_result;

    input_string = "The quick brown fox jumps over the lazy dog";
    input_length = ft_strlen_size_t(input_string);
    compressed_length = 0;
    compressed_buffer = ft_compress(reinterpret_cast<const unsigned char *>(input_string), input_length, &compressed_length);
    if (!compressed_buffer)
        return (0);
    decompressed_length = 0;
    decompressed_buffer = ft_decompress(compressed_buffer, compressed_length, &decompressed_length);
    cma_free(compressed_buffer);
    if (!decompressed_buffer)
        return (0);
    comparison_result = ft_memcmp(decompressed_buffer, input_string, decompressed_length);
    if (comparison_result == 0 && decompressed_length == input_length)
    {
        cma_free(decompressed_buffer);
        return (1);
    }
    cma_free(decompressed_buffer);
    return (0);
}

FT_TEST(test_compression_stream_round_trip, "stream compression round trip")
{
    const char      *input_string;
    int             input_pipe_descriptors[2];
    int             compressed_pipe_descriptors[2];
    unsigned char   *compressed_buffer;
    ssize_t         compressed_size;
    int             decompress_input_pipe_descriptors[2];
    int             decompress_output_pipe_descriptors[2];
    unsigned char   *decompressed_buffer;
    ssize_t         decompressed_size;
    int             comparison_result;

    input_string = "Streaming API example";
    if (pipe(input_pipe_descriptors) != 0)
        return (0);
    if (pipe(compressed_pipe_descriptors) != 0)
    {
        close(input_pipe_descriptors[0]);
        close(input_pipe_descriptors[1]);
        return (0);
    }
    if (su_write(input_pipe_descriptors[1], input_string, ft_strlen_size_t(input_string)) < 0)
    {
        close(input_pipe_descriptors[0]);
        close(input_pipe_descriptors[1]);
        close(compressed_pipe_descriptors[0]);
        close(compressed_pipe_descriptors[1]);
        return (0);
    }
    close(input_pipe_descriptors[1]);
    if (ft_compress_stream(input_pipe_descriptors[0], compressed_pipe_descriptors[1]) != 0)
    {
        close(input_pipe_descriptors[0]);
        close(compressed_pipe_descriptors[0]);
        close(compressed_pipe_descriptors[1]);
        return (0);
    }
    close(input_pipe_descriptors[0]);
    close(compressed_pipe_descriptors[1]);
    compressed_buffer = static_cast<unsigned char*>(cma_malloc(4096));
    if (!compressed_buffer)
    {
        close(compressed_pipe_descriptors[0]);
        return (0);
    }
    compressed_size = su_read(compressed_pipe_descriptors[0], compressed_buffer, 4096);
    close(compressed_pipe_descriptors[0]);
    if (compressed_size <= 0)
    {
        cma_free(compressed_buffer);
        return (0);
    }
    if (pipe(decompress_input_pipe_descriptors) != 0)
    {
        cma_free(compressed_buffer);
        return (0);
    }
    if (pipe(decompress_output_pipe_descriptors) != 0)
    {
        close(decompress_input_pipe_descriptors[0]);
        close(decompress_input_pipe_descriptors[1]);
        cma_free(compressed_buffer);
        return (0);
    }
    if (su_write(decompress_input_pipe_descriptors[1], compressed_buffer,
            static_cast<std::size_t>(compressed_size)) < 0)
    {
        close(decompress_input_pipe_descriptors[0]);
        close(decompress_input_pipe_descriptors[1]);
        close(decompress_output_pipe_descriptors[0]);
        close(decompress_output_pipe_descriptors[1]);
        cma_free(compressed_buffer);
        return (0);
    }
    close(decompress_input_pipe_descriptors[1]);
    cma_free(compressed_buffer);
    if (ft_decompress_stream(decompress_input_pipe_descriptors[0],
            decompress_output_pipe_descriptors[1]) != 0)
    {
        close(decompress_input_pipe_descriptors[0]);
        close(decompress_output_pipe_descriptors[0]);
        close(decompress_output_pipe_descriptors[1]);
        return (0);
    }
    close(decompress_input_pipe_descriptors[0]);
    close(decompress_output_pipe_descriptors[1]);
    decompressed_buffer = static_cast<unsigned char*>(cma_malloc(4096));
    if (!decompressed_buffer)
    {
        close(decompress_output_pipe_descriptors[0]);
        return (0);
    }
    decompressed_size = su_read(decompress_output_pipe_descriptors[0],
        decompressed_buffer, 4096);
    close(decompress_output_pipe_descriptors[0]);
    if (decompressed_size <= 0)
    {
        cma_free(decompressed_buffer);
        return (0);
    }
    comparison_result = ft_memcmp(decompressed_buffer, input_string,
        static_cast<std::size_t>(decompressed_size));
    if (comparison_result == 0 && decompressed_size ==
        static_cast<ssize_t>(ft_strlen_size_t(input_string)))
    {
        cma_free(decompressed_buffer);
        return (1);
    }
    cma_free(decompressed_buffer);
    return (0);
}

FT_TEST(test_base64_round_trip, "base64 round trip")
{
    const char      *input_string;
    unsigned char   *encoded_buffer;
    unsigned char   *decoded_buffer;
    std::size_t     input_length;
    std::size_t     encoded_length;
    std::size_t     decoded_length;
    int             comparison_result;

    input_string = "Base64 encoding";
    input_length = ft_strlen_size_t(input_string);
    encoded_length = 0;
    encoded_buffer = ft_base64_encode(reinterpret_cast<const unsigned char *>(input_string), input_length, &encoded_length);
    if (!encoded_buffer)
        return (0);
    decoded_length = 0;
    decoded_buffer = ft_base64_decode(encoded_buffer, encoded_length, &decoded_length);
    cma_free(encoded_buffer);
    if (!decoded_buffer)
        return (0);
    comparison_result = ft_memcmp(decoded_buffer, input_string, decoded_length);
    if (comparison_result == 0 && decoded_length == input_length)
    {
        cma_free(decoded_buffer);
        return (1);
    }
    cma_free(decoded_buffer);
    return (0);
}

FT_TEST(test_base64_encode_null_terminator, "base64 encode appends null terminator")
{
    const char      *input_string;
    const char      *expected_encoding;
    unsigned char   *encoded_buffer;
    std::size_t     input_length;
    std::size_t     encoded_length;
    std::size_t     expected_length;

    input_string = "hi";
    expected_encoding = "aGk=";
    input_length = 2;
    encoded_length = 0;
    encoded_buffer = ft_base64_encode(reinterpret_cast<const unsigned char *>(input_string), input_length, &encoded_length);
    if (!encoded_buffer)
        return (0);
    expected_length = ft_strlen_size_t(expected_encoding);
    if (encoded_length != expected_length)
    {
        cma_free(encoded_buffer);
        return (0);
    }
    if (ft_strncmp(reinterpret_cast<const char *>(encoded_buffer), expected_encoding, expected_length) != 0)
    {
        cma_free(encoded_buffer);
        return (0);
    }
    if (encoded_buffer[encoded_length] != '\0')
    {
        cma_free(encoded_buffer);
        return (0);
    }
    cma_free(encoded_buffer);
    return (1);
}

FT_TEST(test_base64_decode_ignores_whitespace, "base64 decode ignores whitespace")
{
    const char      *input_string;
    const char      *expected_output;
    unsigned char   *decoded_buffer;
    std::size_t     decoded_length;
    std::size_t     expected_length;

    input_string = "SGVs bG8s\nIHdvcmxkIQ==\r\n";
    expected_output = "Hello, world!";
    decoded_length = 0;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(input_string),
        ft_strlen_size_t(input_string), &decoded_length);
    if (!decoded_buffer)
        return (0);
    expected_length = ft_strlen_size_t(expected_output);
    if (decoded_length != expected_length)
    {
        cma_free(decoded_buffer);
        return (0);
    }
    if (ft_memcmp(decoded_buffer, expected_output, expected_length) != 0)
    {
        cma_free(decoded_buffer);
        return (0);
    }
    cma_free(decoded_buffer);
    return (1);
}

FT_TEST(test_base64_decode_paddingless_tail, "base64 decode paddingless tail")
{
    const char      *input_string;
    const char      *expected_output;
    unsigned char   *decoded_buffer;
    std::size_t     decoded_length;
    std::size_t     expected_length;

    input_string = "YW55IGNhcm5hbCBwbGVhc3Vy";
    expected_output = "any carnal pleasur";
    decoded_length = 0;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(input_string),
        ft_strlen_size_t(input_string), &decoded_length);
    if (!decoded_buffer)
        return (0);
    expected_length = ft_strlen_size_t(expected_output);
    if (decoded_length != expected_length)
    {
        cma_free(decoded_buffer);
        return (0);
    }
    if (ft_memcmp(decoded_buffer, expected_output, expected_length) != 0)
    {
        cma_free(decoded_buffer);
        return (0);
    }
    cma_free(decoded_buffer);
    return (1);
}

FT_TEST(test_base64_invalid_character, "base64 invalid character")
{
    const char      *invalid_input;
    unsigned char   *decoded_buffer;
    std::size_t     decoded_length;

    invalid_input = "AAA!";
    decoded_length = 42;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(invalid_input),
        ft_strlen_size_t(invalid_input), &decoded_length);
    if (decoded_buffer)
    {
        cma_free(decoded_buffer);
        return (0);
    }
    if (decoded_length != 0)
        return (0);
    return (1);
}
