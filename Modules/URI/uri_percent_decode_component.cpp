#include "uri_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

uint8_t *uri_percent_decode_component(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    uint8_t *output;
    ft_size_t input_index;
    ft_size_t output_index;
    int32_t high_value;
    int32_t low_value;

    if (output_size != ft_nullptr)
        *output_size = 0;
    if (input == ft_nullptr && input_size != 0)
    {
        uri_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    output = static_cast<uint8_t *>(cma_malloc(input_size + 1));
    if (output == ft_nullptr)
    {
        uri_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        if (input[input_index] == '%')
        {
            if (input_index + 2 >= input_size)
            {
                cma_free(output);
                uri_set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            high_value = uri_hex_value(input[input_index + 1]);
            low_value = uri_hex_value(input[input_index + 2]);
            if (high_value < 0 || low_value < 0)
            {
                cma_free(output);
                uri_set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            output[output_index++] = static_cast<uint8_t>(
                    (high_value << 4) | low_value);
            input_index += 3;
        }
        else
            output[output_index++] = static_cast<uint8_t>(input[input_index++]);
    }
    output[output_index] = '\0';
    if (output_size != ft_nullptr)
        *output_size = output_index;
    uri_set_error(FT_ERR_SUCCESS);
    return (output);
}
