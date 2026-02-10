#include "CMA.hpp"
#include "cma_internal.hpp"

int32_t cma_set_thread_safety(bool enable)
{
    if (enable)
        return (cma_enable_thread_safety());
    return (cma_disable_thread_safety());
}
