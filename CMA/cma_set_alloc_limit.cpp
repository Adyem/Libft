#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    int entry_errno;
    int current_errno;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_acquired = false;
    if (cma_lock_allocator(&lock_acquired) != 0)
        return ;
    if (lock_acquired)
        ft_errno = entry_errno;
    g_cma_alloc_limit = limit;
    if (lock_acquired)
    {
        current_errno = ft_errno;
        cma_unlock_allocator(lock_acquired);
        ft_errno = current_errno;
        return ;
    }
    cma_unlock_allocator(lock_acquired);
    return ;
}
