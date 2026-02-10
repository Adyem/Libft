#include "advanced.hpp"
#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

void* adv_memdup(const void* source, size_t size)
{
    if (size == 0)
        return (cma_malloc(0));
    if (source == ft_nullptr)
        return (ft_nullptr);
    void *duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
        return (ft_nullptr);
    unsigned char *dest = static_cast<unsigned char *>(duplicate);
    const unsigned char *src = static_cast<const unsigned char *>(source);
    for (size_t index = 0; index < size; ++index)
        dest[index] = src[index];
    return (duplicate);
}
