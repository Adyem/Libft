#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <stdarg.h>

char    *cma_strjoin_multiple(int count, ...)
{
    if (count <= 0)
        return (ft_nullptr);
    va_list args;
    va_start(args, count);
    size_t total_length = 0;
    int argument_index = 0;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        if (current_string)
            total_length += ft_strlen(current_string);
        ++argument_index;
    }
    va_end(args);
    char *result = static_cast<char*>(cma_malloc(total_length + 1));
    if (!result)
        return (ft_nullptr);
    va_start(args, count);
    size_t result_index = 0;
    argument_index = 0;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        if (current_string)
        {
            size_t current_length = ft_strlen(current_string);
            ft_memcpy(result + result_index, current_string, current_length);
            result_index += current_length;
        }
        ++argument_index;
    }
    va_end(args);
    result[result_index] = '\0';
    return (result);
}
