#include "logger_internal.hpp"

bool ft_log_get_color()
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (g_use_color);
}
