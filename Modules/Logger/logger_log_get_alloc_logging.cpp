#include "logger_internal.hpp"

ft_bool ft_log_get_alloc_logging()
{
    if (g_logger)
        return (g_logger->get_alloc_logging());
    return (FT_FALSE);
}
