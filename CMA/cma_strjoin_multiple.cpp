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
    size_t total_len = 0;
    for (int i = 0; i < count; ++i)
    {
        const char *str = va_arg(args, const char *);
        if (str)
            total_len += ft_strlen(str);
    }
    va_end(args);
    char *result = static_cast<char*>(cma_malloc(total_len + 1));
    if (!result)
        return (ft_nullptr);
    va_start(args, count);
    size_t index = 0;
    for (int i = 0; i < count; ++i)
    {
        const char *str = va_arg(args, const char *);
        if (str)
        {
            size_t len = ft_strlen(str);
            ft_memcpy(result + index, str, len);
            index += len;
        }
    }
    va_end(args);
    result[index] = '\0';
    return (result);
}
