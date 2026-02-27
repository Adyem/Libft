#include "advanced.hpp"
#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

char *adv_strndup(const char *string, ft_size_t maximum_length)
{
    if (string == ft_nullptr)
        return (ft_nullptr);
    ft_size_t copy_length = 0;
    while (copy_length < maximum_length && string[copy_length] != '\0')
        copy_length += 1;
    char *duplicate = static_cast<char *>(cma_malloc(copy_length + 1));
    if (duplicate == ft_nullptr)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < copy_length)
    {
        duplicate[index] = string[index];
        ++index;
    }
    duplicate[copy_length] = '\0';
    return (duplicate);
}
