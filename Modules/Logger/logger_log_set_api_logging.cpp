#include "logger_internal.hpp"

void ft_log_set_api_logging(ft_bool enable)
{
    if (g_logger)
    {
        g_logger->set_api_logging(enable);
        return ;
    }
    return ;
}
