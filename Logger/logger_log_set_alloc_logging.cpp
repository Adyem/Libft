#include "logger_internal.hpp"

void ft_log_set_alloc_logging(bool enable)
{
    if (g_logger)
    {
        g_logger->set_alloc_logging(enable);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
