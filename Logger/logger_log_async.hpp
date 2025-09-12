#ifndef LOGGER_LOG_ASYNC_HPP
#define LOGGER_LOG_ASYNC_HPP

#include <cstdarg>
#include "logger.hpp"

void ft_log_enable_async(bool enable);
void ft_log_enqueue(t_log_level level, const char *fmt, va_list args);

#endif
