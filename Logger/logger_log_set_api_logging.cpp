#include "logger_internal.hpp"

void ft_log_set_api_logging(bool enable)
{
    if (g_logger)
    {
        g_logger->set_api_logging(enable);
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}
