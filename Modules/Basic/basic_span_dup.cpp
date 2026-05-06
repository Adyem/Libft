#include "basic.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

char *ft_span_dup(const char *buffer, ft_size_t length)
{
    if (!buffer && length != 0)
        return (ft_nullptr);
    char *duplicate = static_cast<char *>(cma_malloc(length + 1));
    if (duplicate == ft_nullptr)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < length)
    {
        duplicate[index] = buffer[index];
        ++index;
    }
    duplicate[length] = '\0';
    return (duplicate);
}
