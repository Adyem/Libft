#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/test_runner.hpp"
#include "../System_utils/system_utils.hpp"
#include <unistd.h>

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
