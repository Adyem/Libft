#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"

std::size_t cma_alloc_size(const void *ptr)
{
    if (!ptr)
        return (0);
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    const Block *block = reinterpret_cast<const Block*>(
        static_cast<const char*>(ptr) - sizeof(Block));
    if (block->magic != MAGIC_NUMBER)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        return (0);
    }
    std::size_t result = block->size;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    return (result);
}
