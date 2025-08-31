#include "logger_internal.hpp"

const char *ft_level_to_str(t_log_level level)
{
    switch (level)
    {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "";
    }
}
