#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_alloc_limit(ft_size_t limit)
{
    int error_code = FT_ERR_SUCCESSS;
    bool lock_acquired = false;

    error_code = cma_lock_allocator(&lock_acquired);
    if (error_code == FT_ERR_SUCCESSS)
        g_cma_alloc_limit = limit;
    if (lock_acquired)
    {
        cma_unlock_allocator(lock_acquired);
        lock_acquired = false;
    }
    cma_record_operation_error(error_code);
    return ;
}
