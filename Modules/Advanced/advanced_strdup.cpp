#include "advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"


char    *adv_strdup(const char *string)
{
    if (string == ft_nullptr)
        return (ft_nullptr);
    ft_size_t length = 0;
    while (string[length] != '\0')
    {
        if (length >= FT_SYSTEM_SIZE_MAX)
            return (ft_nullptr);
        length += 1;
    }
    ft_size_t allocation_size = length + 1;
    char *new_string = static_cast<char *>(cma_malloc(allocation_size));
    if (new_string == ft_nullptr)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < length)
    {
        new_string[index] = string[index];
        index += 1;
    }
    new_string[index] = '\0';
    return (new_string);
}
