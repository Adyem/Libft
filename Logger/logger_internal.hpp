#ifndef LOGGER_INTERNAL_HPP
#define LOGGER_INTERNAL_HPP

#include <cstdarg>
#include "../Template/vector.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "logger.hpp"
#include "../PThread/mutex.hpp"

extern ft_logger *g_logger;
extern t_log_level g_level;
extern bool g_async_running;
extern bool g_use_color;

typedef void (*t_log_sink)(const char *message, void *user_data);

typedef ssize_t (*t_network_send_function)(int socket_fd, const void *buffer, size_t length, int flags);

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

struct s_network_sink
{
    int                         socket_fd;
    t_network_send_function     send_function;
};

extern ft_vector<s_log_sink> g_sinks;
extern pt_mutex g_sinks_mutex;

void ft_log_rotate(s_file_sink *sink);
void ft_file_sink(const char *message, void *user_data);
void ft_network_sink(const char *message, void *user_data);
const char *ft_level_to_str(t_log_level level);
void ft_log_vwrite(t_log_level level, const char *fmt, va_list args);

#endif
