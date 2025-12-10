#include "logger_internal.hpp"

void ft_log_set_color(bool enable)
{
    g_use_color = enable;
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

