#include <cstddef>
#include "CMA.hpp"
#include "cma_internal.hpp"

void    cma_set_alloc_limit(std::size_t limit)
{
    g_cma_alloc_limit = limit;
    return ;
}
