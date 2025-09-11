#ifndef LOGGER_INTERNAL_HPP
#define LOGGER_INTERNAL_HPP

#include <cstdarg>
#include <vector>
#include "../CPP_class/class_string_class.hpp"
#include "../Errno/errno.hpp"
#include "logger.hpp"

extern ft_logger *g_logger;
extern t_log_level g_level;
extern bool g_async_running;

typedef void (*t_log_sink)(const char *message, void *user_data);

struct s_log_sink
{
    t_log_sink function;
    void      *user_data;
};

struct s_file_sink
{
    int       fd;
    ft_string path;
    size_t    max_size;
};

extern std::vector<s_log_sink> g_sinks;

void ft_log_rotate(s_file_sink *sink);
void ft_file_sink(const char *message, void *user_data);
const char *ft_level_to_str(t_log_level level);
void ft_log_vwrite(t_log_level level, const char *fmt, va_list args);
void ft_log_enqueue(t_log_level level, const char *fmt, va_list args);

#endif
