#include <stddef.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "CMA.hpp"

char    *cma_strdup(const char *string)
{
    size_t      measured_length_raw;
    ft_size_t   length;
    char        *new_string;
    ft_size_t   index;
    ft_size_t   allocation_size;
    int         error_code;

    if (!string)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    measured_length_raw = ft_strlen_size_t(string);
    error_code = ft_global_error_stack_pop_newest();
    if (measured_length_raw >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    length = static_cast<ft_size_t>(measured_length_raw);
    allocation_size = length + static_cast<ft_size_t>(1);
    new_string = static_cast<char *>(cma_malloc(allocation_size));
    error_code = ft_global_error_stack_pop_newest();
    if (!new_string)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    index = 0;
    while (index < length)
    {
        new_string[index] = string[index];
        index++;
    }
    new_string[index] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (new_string);
}
