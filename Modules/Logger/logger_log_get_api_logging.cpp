#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

ft_bool ft_log_get_api_logging()
{
    if (g_logger)
    {
        ft_bool api_logging;

        api_logging = g_logger->get_api_logging();
        return (api_logging);
    }
    return (FT_FALSE);
}
