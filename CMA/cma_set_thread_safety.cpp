#include "CMA.hpp"
#include "cma_internal.hpp"

void    cma_set_thread_safety(bool enable)
{
    g_cma_thread_safe = enable;
    return ;
}

