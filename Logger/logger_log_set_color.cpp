#include "logger_internal.hpp"

void ft_log_set_color(bool enable)
{
    g_use_color = enable;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}
