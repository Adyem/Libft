#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t ft_strnlen(const char *string, size_t maximum_length)
{
    size_t index;
    int error_code;

    if (string == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    index = 0;
    while (index < maximum_length && string[index] != '\0')
    {
        index++;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (index);
}
