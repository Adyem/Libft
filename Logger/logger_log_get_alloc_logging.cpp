#include "logger_internal.hpp"

bool ft_log_get_alloc_logging()
{
    if (g_logger)
        return (g_logger->get_alloc_logging());
    ft_errno = ER_SUCCESS;
    return (false);
}

