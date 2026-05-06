#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

char *ft_strmapi(const char *string, char (*function)(uint32_t, char))
{
    if (!string || !function)
        return (ft_nullptr);
    ft_size_t length = ft_strlen_size_t(string);
    char *result = static_cast<char*>(cma_malloc(length + 1));
    if (!result)
        return (ft_nullptr);
    uint32_t index = 0;
    while (index < length)
    {
        result[index] = function(index, string[index]);
        ++index;
    }
    result[length] = '\0';
    return (result);
}
