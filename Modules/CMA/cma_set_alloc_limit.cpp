#include "CMA.hpp"
#include "cma_internal.hpp"

int32_t cma_set_alloc_limit(ft_size_t limit)
{
    g_cma_alloc_limit = limit;
    return (FT_ERR_SUCCESS);
}
