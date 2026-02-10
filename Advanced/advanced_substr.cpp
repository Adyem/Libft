#include <cstddef>
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

char *adv_substr(const char *source, unsigned int start, size_t length)
{
    if (source == ft_nullptr)
        return (ft_nullptr);
    ft_size_t source_length = safe_strlen(source);
    ft_size_t start_index = start;
    if (start_index >= source_length)
    {
        char *empty_string = static_cast<char *>(cma_malloc(1));
        if (empty_string == ft_nullptr)
            return (ft_nullptr);
        empty_string[0] = '\0';
        return (empty_string);
    }
    ft_size_t available = source_length - start_index;
    ft_size_t desired = static_cast<ft_size_t>(length);
    if (desired > available)
        desired = available;
    if (desired >= FT_SYSTEM_SIZE_MAX)
        return (ft_nullptr);
    char *result = static_cast<char *>(cma_malloc(desired + 1));
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < desired)
    {
        result[index] = source[start_index + index];
        index += 1;
    }
    result[index] = '\0';
    return (result);
}
