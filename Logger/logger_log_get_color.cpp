#include "logger_internal.hpp"

bool ft_log_get_color()
{
    ft_errno = ER_SUCCESS;
    return (g_use_color);
}

