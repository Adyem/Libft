#ifndef LOGGER_INTERNAL_HPP
#define LOGGER_INTERNAL_HPP

#include <cstdarg>
#include <string>
#include "logger.hpp"

extern ft_logger *g_logger;
extern t_log_level g_level;
extern int g_fd;
extern std::string g_path;
extern size_t g_max_size;

void ft_log_rotate();
const char *ft_level_to_str(t_log_level level);
void ft_log_vwrite(t_log_level level, const char *fmt, va_list args);

#endif
