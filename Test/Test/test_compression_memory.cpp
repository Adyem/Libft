#include "../../Compression/compression.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Template/vector.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_compress_string_to_vector_round_trip, "ft_compress_string_to_vector round trips string payloads")
{
    ft_string                  input("In-memory compression helper");
    ft_vector<unsigned char>   compressed;
    ft_string                  output;
    int                        result;
    int                        comparison_result;

    ft_errno = ER_SUCCESS;
    result = ft_compress_string_to_vector(input, compressed);
    if (result != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    result = ft_decompress_vector_to_string(compressed, output);
    if (result != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    if (output.size() != input.size())
        return (0);
    comparison_result = ft_memcmp(output.c_str(), input.c_str(), input.size());
    if (comparison_result != 0)
        return (0);
    return (1);
}

FT_TEST(test_ft_compress_vector_to_vector_round_trip, "ft_compress_vector_to_vector handles binary payloads")
{
    unsigned char              sample_bytes[6];
    ft_vector<unsigned char>   input;
    ft_vector<unsigned char>   compressed;
    ft_vector<unsigned char>   output;
    std::size_t                sample_count;
    std::size_t                index;
    int                        result;
    std::size_t                input_size;
    std::size_t                output_size;
    const unsigned char        *input_data;
    const unsigned char        *output_data;

    sample_bytes[0] = 0;
    sample_bytes[1] = 1;
    sample_bytes[2] = 0;
    sample_bytes[3] = 255;
    sample_bytes[4] = 128;
    sample_bytes[5] = 64;
    sample_count = sizeof(sample_bytes) / sizeof(sample_bytes[0]);
    index = 0;
    while (index < sample_count)
    {
        input.push_back(sample_bytes[index]);
        if (input.get_error() != ER_SUCCESS)
            return (0);
        index++;
    }
    ft_errno = ER_SUCCESS;
    result = ft_compress_vector_to_vector(input, compressed);
    if (result != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    result = ft_decompress_vector_to_vector(compressed, output);
    if (result != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    input_size = input.size();
    if (input.get_error() != ER_SUCCESS)
        return (0);
    output_size = output.size();
    if (output.get_error() != ER_SUCCESS)
        return (0);
    if (input_size != output_size)
        return (0);
    if (input_size == 0)
        return (1);
    input_data = input.begin();
    if (input.get_error() != ER_SUCCESS)
        return (0);
    output_data = output.begin();
    if (output.get_error() != ER_SUCCESS)
        return (0);
    if (ft_memcmp(input_data, output_data, input_size) != 0)
        return (0);
    return (1);
}

FT_TEST(test_ft_decompress_vector_to_string_reports_allocator_failure, "ft_decompress_vector_to_string reports allocation failures")
{
    ft_string                  input("allocator failure exercise");
    ft_vector<unsigned char>   compressed;
    ft_string                  output;
    int                        result;

    ft_errno = ER_SUCCESS;
    if (ft_compress_string_to_vector(input, compressed) != 0)
        return (0);
    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    result = ft_decompress_vector_to_string(compressed, output);
    cma_set_alloc_limit(0);
    if (result == 0)
        return (0);
    if (ft_errno != FT_ERR_NO_MEMORY)
        return (0);
    return (1);
}
