#include "logger_internal.hpp"
#include "../CMA/CMA.hpp"

void ft_log_set_alloc_logging(ft_bool enable)
{
    if (g_logger)
    {
        g_logger->set_alloc_logging(enable);
        return ;
    }
    return ;
}
