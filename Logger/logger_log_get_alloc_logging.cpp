#include "logger_internal.hpp"

bool ft_log_get_alloc_logging()
{
    if (g_logger)
    {
        bool alloc_logging;

        alloc_logging = g_logger->get_alloc_logging();
        return (alloc_logging);
    }
    return (false);
}
