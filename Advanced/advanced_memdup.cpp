#include "advanced.hpp"
#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

void* adv_memdup(const void* source, ft_size_t size)
{
    if (size == 0)
        return (cma_malloc(0));
    if (source == ft_nullptr)
        return (ft_nullptr);
    void *duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
        return (ft_nullptr);
    unsigned char *destination = static_cast<unsigned char *>(duplicate);
    const unsigned char *source_bytes = static_cast<const unsigned char *>(source);
    ft_size_t index = 0;
    while (index < size)
    {
        destination[index] = source_bytes[index];
        ++index;
    }
    return (duplicate);
}
