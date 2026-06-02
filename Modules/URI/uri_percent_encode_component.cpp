#include "uri_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

char *uri_percent_encode_component(const uint8_t *input, ft_size_t input_size,
    ft_bool keep_slash)
{
    ft_size_t output_size;
    ft_size_t input_index;
    ft_size_t output_index;
    char *output;

    if (input == ft_nullptr && input_size != 0)
    {
        uri_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (uri_mul_overflows(input_size, 3) == FT_TRUE)
    {
        uri_set_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    output_size = input_size * 3;
    output = static_cast<char *>(cma_malloc(output_size + 1));
    if (output == ft_nullptr)
    {
        uri_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        if (uri_is_unreserved(input[input_index]) == FT_TRUE
            || (keep_slash == FT_TRUE && input[input_index] == '/'))
            output[output_index++] = static_cast<char>(input[input_index]);
        else
        {
            output[output_index++] = '%';
            output[output_index++] = uri_hex_character(
                    static_cast<uint8_t>(input[input_index] >> 4));
            output[output_index++] = uri_hex_character(
                    static_cast<uint8_t>(input[input_index] & 0x0FU));
        }
        input_index++;
    }
    output[output_index] = '\0';
    uri_set_error(FT_ERR_SUCCESS);
    return (output);
}
