#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    int entry_errno;
    int lock_result;
    int current_errno;

    entry_errno = ft_errno;
    if (g_cma_thread_safe)
    {
        lock_result = g_malloc_mutex.lock(THREAD_ID);
        if (lock_result != FT_SUCCESS)
            return ;
        ft_errno = entry_errno;
    }
    g_cma_alloc_limit = limit;
    if (g_cma_thread_safe)
    {
        current_errno = ft_errno;
        lock_result = g_malloc_mutex.unlock(THREAD_ID);
        if (lock_result != FT_SUCCESS)
            return ;
        ft_errno = current_errno;
    }
    return ;
}
