#include "logger_internal.hpp"

void ft_log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_ERROR, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}
