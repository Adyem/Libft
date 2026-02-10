#include <cstddef>
#include <cstdarg>
#include "advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

static ft_size_t safe_strlen(const char *string)
{
    if (string == ft_nullptr)
        return (0);
    ft_size_t length = 0;
    while (string[length] != '\0')
    {
        if (length >= FT_SYSTEM_SIZE_MAX)
            return (FT_SYSTEM_SIZE_MAX);
        length += 1;
    }
    return (length);
}

char *adv_strjoin_multiple(int count, ...)
{
    if (count <= 0)
        return (ft_nullptr);
    if (static_cast<ft_size_t>(count) > FT_SYSTEM_SIZE_MAX / sizeof(ft_size_t))
        return (ft_nullptr);
    ft_size_t *length_cache = static_cast<ft_size_t *>(
            cma_malloc(static_cast<ft_size_t>(count) * sizeof(ft_size_t)));
    if (length_cache == ft_nullptr)
        return (ft_nullptr);
    va_list args;
    va_start(args, count);
    ft_size_t total_length = 0;
    for (int i = 0; i < count; ++i)
    {
        const char *current = va_arg(args, const char *);
        ft_size_t length = safe_strlen(current);
        if (length == FT_SYSTEM_SIZE_MAX)
        {
            va_end(args);
            cma_free(length_cache);
            return (ft_nullptr);
        }
        if (total_length > FT_SYSTEM_SIZE_MAX - length)
        {
            va_end(args);
            cma_free(length_cache);
            return (ft_nullptr);
        }
        length_cache[i] = length;
        total_length += length;
    }
    va_end(args);
    ft_size_t allocation_size = total_length + 1;
    char *result = static_cast<char *>(cma_malloc(allocation_size));
    if (result == ft_nullptr)
    {
        cma_free(length_cache);
        return (ft_nullptr);
    }
    va_start(args, count);
    ft_size_t write_index = 0;
    for (int i = 0; i < count; ++i)
    {
        const char *current = va_arg(args, const char *);
        ft_size_t length = length_cache[i];
        for (ft_size_t j = 0; j < length; ++j)
            result[write_index++] = current[j];
    }
    va_end(args);
    cma_free(length_cache);
    result[write_index] = '\0';
    return (result);
}
