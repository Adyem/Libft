#include <zlib.h>
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "compression.hpp"

unsigned char    *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size)
{
    uLongf          zlib_bound;
    uLongf          actual_size;
    unsigned char   *result_buffer;
    unsigned char   *resized_buffer;
    uint32_t        original_size;
    int             zlib_status;

    if (!input_buffer || !compressed_size)
        return (ft_nullptr);
    zlib_bound = compressBound(static_cast<uLong>(input_size));
    result_buffer = static_cast<unsigned char *>(cma_malloc(zlib_bound + sizeof(uint32_t)));
    if (!result_buffer)
        return (ft_nullptr);
    actual_size = zlib_bound;
    zlib_status = compress2(result_buffer + sizeof(uint32_t), &actual_size, input_buffer, static_cast<uLong>(input_size), Z_BEST_COMPRESSION);
    if (zlib_status != Z_OK)
    {
        cma_free(result_buffer);
        return (ft_nullptr);
    }
    original_size = static_cast<uint32_t>(input_size);
    ft_memcpy(result_buffer, &original_size, sizeof(uint32_t));
    *compressed_size = actual_size + sizeof(uint32_t);
    resized_buffer = static_cast<unsigned char *>(cma_realloc(result_buffer, *compressed_size));
    if (resized_buffer)
        result_buffer = resized_buffer;
    return (result_buffer);
}

unsigned char    *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size)
{
    uint32_t        expected_size;
    uLongf          actual_size;
    unsigned char   *result_buffer;
    int             zlib_status;

    if (!input_buffer || input_size < sizeof(uint32_t) || !decompressed_size)
        return (ft_nullptr);
    ft_memcpy(&expected_size, input_buffer, sizeof(uint32_t));
    result_buffer = static_cast<unsigned char *>(cma_malloc(expected_size));
    if (!result_buffer)
        return (ft_nullptr);
    actual_size = static_cast<uLongf>(expected_size);
    zlib_status = uncompress(result_buffer, &actual_size, input_buffer + sizeof(uint32_t), input_size - sizeof(uint32_t));
    if (zlib_status != Z_OK || actual_size != expected_size)
    {
        cma_free(result_buffer);
        return (ft_nullptr);
    }
    *decompressed_size = actual_size;
    return (result_buffer);
}

unsigned char    *ft_compress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size)
{
    return (compress_buffer(input_buffer, input_size, compressed_size));
}

unsigned char    *ft_decompress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size)
{
    return (decompress_buffer(input_buffer, input_size, decompressed_size));
}
