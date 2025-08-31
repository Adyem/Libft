#include "logger_internal.hpp"

void ft_log_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ft_log_vwrite(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}
