#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Basic/limits.hpp"

int32_t cma_set_thread_safety(ft_bool enable)
{
    if (enable == FT_TRUE)
        return (cma_enable_thread_safety());
    return (cma_disable_thread_safety());
}
