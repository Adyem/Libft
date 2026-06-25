#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

void ft_log_set_level(t_log_level level)
{
    g_level = level;
    return ;
}
