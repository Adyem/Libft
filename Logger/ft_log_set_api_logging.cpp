#include "logger.hpp"

void ft_log_set_api_logging(bool enable)
{
    if (g_logger)
        g_logger->set_api_logging(enable);
}

