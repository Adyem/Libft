#include <cstddef>
#include "CMA.hpp"
#include "../CPP_class/nullptr.hpp"

void *cma_aligned_alloc(std::size_t alignment, std::size_t size)
{
    if ((alignment & (alignment - 1)) != 0 || alignment < sizeof(void *))
        return (ft_nullptr);
    std::size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    return (cma_malloc(aligned_size));
}
