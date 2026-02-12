#include "CMA.hpp"
#include "cma_internal.hpp"

int32_t cma_set_alloc_limit(ft_size_t limit)
{
    bool lock_acquired = false;
    int32_t error_code = cma_lock_allocator(&lock_acquired);

    if (error_code == FT_ERR_SUCCESS)
        g_cma_alloc_limit = limit;
    if (lock_acquired)
        cma_unlock_allocator(lock_acquired);
    return (error_code);
}
