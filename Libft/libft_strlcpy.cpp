#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static size_t report_strlcpy_result(int error_code, size_t return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

size_t ft_strlcpy(char *destination, const char *source, size_t buffer_size)
{
    size_t source_length;

    if (source == ft_nullptr)
        return (report_strlcpy_result(FT_ERR_INVALID_ARGUMENT, 0));
    source_length = 0;
    if (buffer_size == 0)
    {
        while (source[source_length] != '\0')
        {
            source_length++;
        }
        return (report_strlcpy_result(FT_ERR_SUCCESSS, source_length));
    }
    if (destination == ft_nullptr)
        return (report_strlcpy_result(FT_ERR_INVALID_ARGUMENT, 0));
    while (source_length < buffer_size - 1 && source[source_length] != '\0')
    {
        destination[source_length] = source[source_length];
        source_length++;
    }
    if (source_length < buffer_size)
        destination[source_length] = '\0';
    while (source[source_length] != '\0')
    {
        source_length++;
    }
    return (report_strlcpy_result(FT_ERR_SUCCESSS, source_length));
}
