#include "logger_internal.hpp"

void ft_log_info(const char *format_string, ...)
{
    va_list argument_list;
    va_start(argument_list, format_string);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_INFO, format_string, argument_list);
    else
        ft_log_vwrite(LOG_LEVEL_INFO, format_string, argument_list);
    va_end(argument_list);
}
