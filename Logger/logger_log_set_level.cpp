#include "logger_internal.hpp"

void ft_log_set_level(t_log_level level)
{
    g_level = level;
    ft_errno = ER_SUCCESS;
    return ;
}
