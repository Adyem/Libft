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
    if (static_cast<size_t>(count) > SIZE_MAX / sizeof(size_t))
    {
        ft_errno = FT_ERANGE;
        return (ft_nullptr);
    }
    size_t *cached_lengths = static_cast<size_t*>(cma_malloc(static_cast<size_t>(count) * sizeof(size_t)));
    if (!cached_lengths)
    {
        ft_errno = FT_EALLOC;
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
        size_t current_length = 0;
        if (current_string)
        {
            int string_length = ft_strlen(current_string);
            if (ft_errno != ER_SUCCESS)
            {
                ft_errno = FT_ERANGE;
                va_end(args);
                cma_free(cached_lengths);
                return (ft_nullptr);
            }
            current_length = static_cast<size_t>(string_length);
            if (total_length > SIZE_MAX - current_length)
            {
                ft_errno = FT_ERANGE;
                va_end(args);
                cma_free(cached_lengths);
                return (ft_nullptr);
            }
            total_length += current_length;
        }
        cached_lengths[argument_index] = current_length;
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
        cma_free(cached_lengths);
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
            size_t current_length = cached_lengths[argument_index];
            if (current_length > 0)
            {
                ft_memcpy(result + result_index, current_string, current_length);
                result_index += current_length;
            }
        }
        ++argument_index;
    }
    va_end(args);
    cma_free(cached_lengths);
    result[result_index] = '\0';
    return (result);
}
