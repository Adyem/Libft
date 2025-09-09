#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"

std::size_t cma_alloc_size(const void *ptr)
{
    if (!ptr)
        return (0);
    const Block *block = reinterpret_cast<const Block*>(
        static_cast<const char*>(ptr) - sizeof(Block));
    if (block->magic != MAGIC_NUMBER)
        return (0);
    return (block->size);
}
