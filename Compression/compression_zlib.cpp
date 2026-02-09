#include <zlib.h>
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "compression.hpp"

static const unsigned char g_compression_empty_input = 0;

static bool compression_vector_operation_failed(void)
{
    int vector_error = ft_global_error_stack_peek_last_error();

    if (vector_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(vector_error);
        return (true);
    }
    return (false);
}

static int compression_string_pop_error(const ft_string &string_value)
{
    unsigned long long operation_id = string_value.last_operation_id();

    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    return (string_value.pop_operation_error(operation_id));
}

static int  compression_store_in_vector(ft_vector<unsigned char> &destination, const unsigned char *buffer, std::size_t size)
{
    unsigned char   *destination_data;

    if (size == 0)
    {
        destination.clear();
        if (compression_vector_operation_failed())
            return (1);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (buffer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    destination.resize(size);
    if (compression_vector_operation_failed())
        return (1);
    destination_data = destination.begin();
    if (compression_vector_operation_failed())
        return (1);
    ft_memcpy(destination_data, buffer, size);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

static int  compression_assign_string(ft_string &destination, const unsigned char *buffer, std::size_t size)
{
    int string_error;

    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(string_error);
        return (1);
    }
    if (size == 0)
    {
        destination.clear();
        string_error = ft_string::last_operation_error();
        if (string_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(string_error);
            return (1);
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (buffer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    destination.assign(reinterpret_cast<const char *>(buffer), size);
        string_error = ft_string::last_operation_error();
        if (string_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(string_error);
            return (1);
        }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

unsigned char    *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size)
{
    uLongf          zlib_bound;
    uLongf          actual_size;
    unsigned char   *result_buffer;
    unsigned char   *resized_buffer;
    uint32_t        original_size;
    int             zlib_status;

    if (!compressed_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    *compressed_size = 0;
    if (!input_buffer)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (input_size > compression_max_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    zlib_bound = compressBound(input_size);
    result_buffer = static_cast<unsigned char *>(cma_malloc(zlib_bound + sizeof(uint32_t)));
    if (!result_buffer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    actual_size = zlib_bound;
    zlib_status = compress2(result_buffer + sizeof(uint32_t), &actual_size, input_buffer, input_size, Z_BEST_COMPRESSION);
    if (zlib_status != Z_OK)
    {
        cma_free(result_buffer);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    original_size = static_cast<uint32_t>(input_size);
    ft_memcpy(result_buffer, &original_size, sizeof(uint32_t));
    *compressed_size = actual_size + sizeof(uint32_t);
    resized_buffer = static_cast<unsigned char *>(cma_realloc(result_buffer, *compressed_size));
    if (resized_buffer)
        result_buffer = resized_buffer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result_buffer);
}

unsigned char    *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size)
{
    uint32_t        expected_size;
    uLongf          actual_size;
    unsigned char   *result_buffer;
    int             zlib_status;

    if (!decompressed_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    *decompressed_size = 0;
    if (!input_buffer || input_size < sizeof(uint32_t))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_memcpy(&expected_size, input_buffer, sizeof(uint32_t));
    if (static_cast<std::size_t>(expected_size) > compression_max_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (input_size - sizeof(uint32_t) > compression_max_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    result_buffer = static_cast<unsigned char *>(cma_malloc(expected_size));
    if (!result_buffer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    actual_size = static_cast<uLongf>(expected_size);
    zlib_status = uncompress(result_buffer, &actual_size, input_buffer + sizeof(uint32_t), input_size - sizeof(uint32_t));
    if (zlib_status != Z_OK || actual_size != expected_size)
    {
        cma_free(result_buffer);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    *decompressed_size = actual_size;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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

int ft_compress_string_to_vector(const ft_string &input, ft_vector<unsigned char> &output)
{
    const unsigned char   *input_buffer;
    std::size_t           input_length;
    unsigned char         *compressed_buffer;
    std::size_t           compressed_size;
    int                   store_status;

    {
        int string_error = compression_string_pop_error(input);

        if (string_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(string_error);
            return (1);
        }
    }
    input_length = input.size();
    input_buffer = reinterpret_cast<const unsigned char *>(input.c_str());
    compressed_size = 0;
    compressed_buffer = ft_compress(input_buffer, input_length, &compressed_size);
    if (!compressed_buffer)
        return (1);
    store_status = compression_store_in_vector(output, compressed_buffer, compressed_size);
    cma_free(compressed_buffer);
    if (store_status != 0)
        return (1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int ft_compress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output)
{
    std::size_t           input_size;
    const unsigned char   *input_buffer;
    unsigned char         *compressed_buffer;
    std::size_t           compressed_size;
    int                   store_status;

    input_size = input.size();
    if (compression_vector_operation_failed())
        return (1);
    if (input_size == 0)
        input_buffer = &g_compression_empty_input;
    else
    {
        input_buffer = input.begin();
        if (compression_vector_operation_failed())
            return (1);
    }
    compressed_size = 0;
    compressed_buffer = ft_compress(input_buffer, input_size, &compressed_size);
    if (!compressed_buffer)
        return (1);
    store_status = compression_store_in_vector(output, compressed_buffer, compressed_size);
    cma_free(compressed_buffer);
    if (store_status != 0)
        return (1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int ft_decompress_vector_to_string(const ft_vector<unsigned char> &input, ft_string &output)
{
    std::size_t           input_size;
    const unsigned char   *input_buffer;
    unsigned char         *decompressed_buffer;
    std::size_t           decompressed_size;
    int                   assign_status;

    input_size = input.size();
    if (compression_vector_operation_failed())
        return (1);
    if (input_size == 0)
        input_buffer = &g_compression_empty_input;
    else
    {
        input_buffer = input.begin();
        if (compression_vector_operation_failed())
            return (1);
    }
    decompressed_size = 0;
    decompressed_buffer = ft_decompress(input_buffer, input_size, &decompressed_size);
    if (!decompressed_buffer)
        return (1);
    assign_status = compression_assign_string(output, decompressed_buffer, decompressed_size);
    cma_free(decompressed_buffer);
    if (assign_status != 0)
        return (1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int ft_decompress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output)
{
    std::size_t           input_size;
    const unsigned char   *input_buffer;
    unsigned char         *decompressed_buffer;
    std::size_t           decompressed_size;
    int                   store_status;

    input_size = input.size();
    if (compression_vector_operation_failed())
        return (1);
    if (input_size == 0)
        input_buffer = &g_compression_empty_input;
    else
    {
        input_buffer = input.begin();
        if (compression_vector_operation_failed())
            return (1);
    }
    decompressed_size = 0;
    decompressed_buffer = ft_decompress(input_buffer, input_size, &decompressed_size);
    if (!decompressed_buffer)
        return (1);
    store_status = compression_store_in_vector(output, decompressed_buffer, decompressed_size);
    cma_free(decompressed_buffer);
    if (store_status != 0)
        return (1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
