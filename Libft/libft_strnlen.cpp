#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static size_t report_strnlen_result(int error_code, size_t return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

size_t ft_strnlen(const char *string, size_t maximum_length)
{
    size_t index;
    int error_code;

    if (string == ft_nullptr)
        return (report_strnlen_result(FT_ERR_INVALID_ARGUMENT, 0));
    index = 0;
    while (index < maximum_length && string[index] != '\0')
    {
        index++;
    }
    error_code = FT_ERR_SUCCESSS;
    return (report_strnlen_result(error_code, index));
}
