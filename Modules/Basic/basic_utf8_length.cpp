#include <cstddef>
#include "basic.hpp"
#include "utf8.hpp"
#include "../Errno/errno.hpp"

int32_t ft_utf8_count(const char *string, ft_size_t *code_point_count_pointer)
{
    int32_t error_code;

    if (!string || !code_point_count_pointer)
        return (FT_ERR_INVALID_POINTER);
    ft_size_t length = ft_strlen_size_t(string);
    ft_size_t index = 0;
    ft_size_t count = 0;
    while (index < length)
    {
        uint32_t code_point = 0;
        ft_size_t sequence_length = 0;
        ft_size_t working_index = index;
        error_code = ft_utf8_next(string, length, &working_index, &code_point, &sequence_length);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        index = working_index;
        ++count;
    }
    *code_point_count_pointer = count;
    return (FT_ERR_SUCCESS);
}
