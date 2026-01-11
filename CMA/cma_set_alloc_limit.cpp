#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    cma_allocator_guard allocator_guard;
    int error_code;

    if (!allocator_guard.is_active())
    {
        g_cma_alloc_limit = limit;
        error_code = allocator_guard.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return ;
    }
    g_cma_alloc_limit = limit;
    allocator_guard.unlock();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return ;
}
