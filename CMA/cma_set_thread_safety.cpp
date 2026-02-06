#include "CMA.hpp"
#include "cma_internal.hpp"

void    cma_set_thread_safety(bool enable)
{
    if (enable)
        cma_enable_thread_safety();
    else
        cma_disable_thread_safety();
    return ;
}
