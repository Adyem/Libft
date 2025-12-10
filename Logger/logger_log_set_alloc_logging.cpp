#include "logger_internal.hpp"

void ft_log_set_alloc_logging(bool enable)
{
    if (g_logger)
    {
        g_logger->set_alloc_logging(enable);
        return ;
    }
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

