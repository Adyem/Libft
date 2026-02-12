#include "logger_internal.hpp"

bool ft_log_get_api_logging()
{
    if (g_logger)
    {
        bool api_logging;

        api_logging = g_logger->get_api_logging();
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (api_logging);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (false);
}
