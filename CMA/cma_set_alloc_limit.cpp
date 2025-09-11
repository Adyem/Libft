#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"

void    cma_set_alloc_limit(std::size_t limit)
{
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    g_cma_alloc_limit = limit;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    return ;
}
