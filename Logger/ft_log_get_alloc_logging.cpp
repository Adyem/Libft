#include "logger.hpp"

bool ft_log_get_alloc_logging()
{
    if (g_logger)
        return g_logger->get_alloc_logging();
    return false;
}

