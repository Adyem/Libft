#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

void ft_log_set_api_logging(ft_bool enable)
{
    if (g_logger)
    {
        g_logger->set_api_logging(enable);
        return ;
    }
    return ;
}
