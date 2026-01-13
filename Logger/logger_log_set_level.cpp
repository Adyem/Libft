#include "logger_internal.hpp"

void ft_log_set_level(t_log_level level)
{
    g_level = level;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
