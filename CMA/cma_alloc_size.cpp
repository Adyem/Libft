#include <cstddef>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

ft_size_t cma_alloc_size(const void *ptr)
{
    if (!ptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    const Block *block = reinterpret_cast<const Block*>(
        static_cast<const char*>(ptr) - sizeof(Block));
    if (block->magic != MAGIC_NUMBER)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = CMA_INVALID_PTR;
        return (0);
    }
    ft_size_t result = block->size;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    ft_errno = ER_SUCCESS;
    return (result);
}
