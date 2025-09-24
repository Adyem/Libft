#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <stdarg.h>

char *ft_strjoin_multiple(int count, ...)
{
    if (count <= 0)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    va_list args;
    va_start(args, count);
    size_t total_length = 0;
    int argument_index = 0;
    ft_errno = ER_SUCCESS;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        if (current_string)
        {
            int string_length = ft_strlen(current_string);
            if (ft_errno != ER_SUCCESS)
            {
                ft_errno = FT_ERANGE;
                va_end(args);
                return (ft_nullptr);
            }
            if (total_length > SIZE_MAX - static_cast<size_t>(string_length))
            {
                ft_errno = FT_ERANGE;
                va_end(args);
                return (ft_nullptr);
            }
            total_length += static_cast<size_t>(string_length);
        }
        ++argument_index;
    }
    va_end(args);
    if (total_length == SIZE_MAX)
    {
        ft_errno = FT_ERANGE;
        return (ft_nullptr);
    }
    char *result = static_cast<char*>(cma_malloc(total_length + 1));
    if (!result)
    {
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    va_start(args, count);
    size_t result_index = 0;
    argument_index = 0;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        if (current_string)
        {
            int string_length = ft_strlen(current_string);
            if (ft_errno != ER_SUCCESS)
            {
                ft_errno = FT_ERANGE;
                va_end(args);
                cma_free(result);
                return (ft_nullptr);
            }
            ft_memcpy(result + result_index, current_string,
                static_cast<size_t>(string_length));
            result_index += static_cast<size_t>(string_length);
        }
        ++argument_index;
    }
    va_end(args);
    result[result_index] = '\0';
    return (result);
}
