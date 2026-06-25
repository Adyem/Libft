#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

const char *ft_level_to_str(t_log_level level)
{
    if (level == LOG_LEVEL_DEBUG)
        return ("DEBUG");
    if (level == LOG_LEVEL_INFO)
        return ("INFO");
    if (level == LOG_LEVEL_WARN)
        return ("WARN");
    if (level == LOG_LEVEL_ERROR)
        return ("ERROR");
    return ("");
}

int32_t ft_log_level_to_severity(t_log_level level)
{
    if (level == LOG_LEVEL_DEBUG)
        return (10);
    if (level == LOG_LEVEL_INFO)
        return (20);
    if (level == LOG_LEVEL_WARN)
        return (30);
    if (level == LOG_LEVEL_ERROR)
        return (40);
    return (FT_ERR_SUCCESS);
}
