#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

void ft_log_set_color(ft_bool enable)
{
    g_use_color = enable;
    return ;
}
