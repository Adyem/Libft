#include "logger_internal.hpp"

bool ft_log_get_api_logging()
{
    if (g_logger)
        return (g_logger->get_api_logging());
    ft_errno = FT_ERR_SUCCESSS;
    return (false);
}

