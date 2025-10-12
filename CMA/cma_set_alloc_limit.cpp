#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    int entry_errno;
    int current_errno;
    entry_errno = ft_errno;
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    if (allocator_guard.lock_acquired())
        ft_errno = entry_errno;
    g_cma_alloc_limit = limit;
    if (allocator_guard.lock_acquired())
    {
        current_errno = ft_errno;
        allocator_guard.unlock();
        ft_errno = current_errno;
        return ;
    }
    allocator_guard.unlock();
    return ;
}
