#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
    {
        g_cma_alloc_limit = limit;
        if (allocator_guard.get_error() == FT_ERR_SUCCESSS)
            ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    g_cma_alloc_limit = limit;
    allocator_guard.unlock();
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
