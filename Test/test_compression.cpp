#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/test_runner.hpp"
#include <cstdio>

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
    char            *input_copy;
    FILE            *input_stream;
    char            *compressed_data;
    std::size_t     compressed_size;
    FILE            *compressed_stream;
    char            *decompressed_data;
    std::size_t     decompressed_size;
    FILE            *decompressed_stream;
    int             comparison_result;

    input_string = "Streaming API example";
    input_copy = cma_strdup(input_string);
    if (!input_copy)
        return (0);
    input_stream = fmemopen(input_copy, ft_strlen_size_t(input_copy), "r");
    if (!input_stream)
    {
        cma_free(input_copy);
        return (0);
    }
    compressed_data = ft_nullptr;
    compressed_size = 0;
    compressed_stream = open_memstream(&compressed_data, &compressed_size);
    if (!compressed_stream)
    {
        fclose(input_stream);
        cma_free(input_copy);
        return (0);
    }
    if (ft_compress_stream(input_stream, compressed_stream) != 0)
    {
        fclose(input_stream);
        fclose(compressed_stream);
        cma_free(input_copy);
        if (compressed_data)
            cma_free(compressed_data);
        return (0);
    }
    fclose(input_stream);
    fclose(compressed_stream);
    cma_free(input_copy);
    decompressed_data = ft_nullptr;
    decompressed_size = 0;
    compressed_stream = fmemopen(compressed_data, compressed_size, "r");
    if (!compressed_stream)
    {
        cma_free(compressed_data);
        return (0);
    }
    decompressed_stream = open_memstream(&decompressed_data, &decompressed_size);
    if (!decompressed_stream)
    {
        fclose(compressed_stream);
        cma_free(compressed_data);
        return (0);
    }
    if (ft_decompress_stream(compressed_stream, decompressed_stream) != 0)
    {
        fclose(compressed_stream);
        fclose(decompressed_stream);
        cma_free(compressed_data);
        if (decompressed_data)
            cma_free(decompressed_data);
        return (0);
    }
    fclose(compressed_stream);
    fclose(decompressed_stream);
    cma_free(compressed_data);
    comparison_result = ft_memcmp(decompressed_data, input_string, decompressed_size);
    if (comparison_result == 0 && decompressed_size == ft_strlen_size_t(input_string))
    {
        cma_free(decompressed_data);
        return (1);
    }
    cma_free(decompressed_data);
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
