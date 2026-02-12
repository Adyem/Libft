#include "logger_internal.hpp"

bool ft_log_get_alloc_logging()
{
    if (g_logger)
    {
        bool alloc_logging;

        alloc_logging = g_logger->get_alloc_logging();
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (alloc_logging);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (false);
}
