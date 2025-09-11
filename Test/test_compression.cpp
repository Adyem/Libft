#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

int test_compression_round_trip(void)
{
    const char      *input_string;
    unsigned char   *compressed_buffer;
    unsigned char   *decompressed_buffer;
    size_t          input_length;
    size_t          compressed_length;
    size_t          decompressed_length;
    int             comparison_result;

    input_string = "The quick brown fox jumps over the lazy dog";
    input_length = ft_strlen(input_string);
    compressed_length = 0;
    compressed_buffer = compress_buffer(reinterpret_cast<const unsigned char *>(input_string), input_length, &compressed_length);
    if (!compressed_buffer)
        return (0);
    decompressed_length = 0;
    decompressed_buffer = decompress_buffer(compressed_buffer, compressed_length, &decompressed_length);
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
