#include <cstddef>
#include "advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

void    *adv_calloc(ft_size_t count, ft_size_t size)
{
    if (count == 0 || size == 0)
        return (ft_nullptr);
    if (size > FT_SYSTEM_SIZE_MAX / count)
        return (ft_nullptr);
    ft_size_t total_size = count * size;
    void *memory_pointer = cma_malloc(total_size);
    if (memory_pointer == ft_nullptr)
        return (ft_nullptr);
    unsigned char *bytes = static_cast<unsigned char *>(memory_pointer);
    ft_size_t index = 0;
    while (index < total_size)
    {
        bytes[index] = 0;
        index += 1;
    }
    return (memory_pointer);
}
