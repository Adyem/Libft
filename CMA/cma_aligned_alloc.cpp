#include <cstddef>
#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

void *cma_aligned_alloc(ft_size_t alignment, ft_size_t size)
{
    if ((alignment & (alignment - 1)) != 0
        || alignment < static_cast<ft_size_t>(sizeof(void *)))
        return (ft_nullptr);
    ft_size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    return (cma_malloc(aligned_size));
}
