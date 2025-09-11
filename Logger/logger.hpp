#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <cstddef>

enum t_log_level {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
};

// Sink management. A sink receives a formatted message and an opaque
// user pointer. Multiple sinks may be registered at once.
typedef void (*t_log_sink)(const char *message, void *user_data);
int     ft_log_add_sink(t_log_sink sink, void *user_data);
void    ft_log_remove_sink(t_log_sink sink, void *user_data);

void    ft_log_set_level(t_log_level level);
int     ft_log_set_file(const char *path, size_t max_size);
void    ft_log_close();
void    ft_log_set_alloc_logging(bool enable);
bool    ft_log_get_alloc_logging();
void    ft_log_set_api_logging(bool enable);
bool    ft_log_get_api_logging();

void ft_log_debug(const char *fmt, ...);
void ft_log_info(const char *fmt, ...);
void ft_log_warn(const char *fmt, ...);
void ft_log_error(const char *fmt, ...);
void ft_log_start_async();
void ft_log_stop_async();

class ft_logger
{
    private:
        bool _alloc_logging;
        bool _api_logging;

    public:
        ft_logger(const char *path = nullptr, size_t max_size = 0,
                  t_log_level level = LOG_LEVEL_INFO) noexcept;
        ~ft_logger() noexcept;

        ft_logger(const ft_logger&) = delete;
        ft_logger &operator=(const ft_logger&) = delete;

        void set_global() noexcept;
        void set_level(t_log_level level) noexcept;
        int  set_file(const char *path, size_t max_size) noexcept;
        int  add_sink(t_log_sink sink, void *user_data) noexcept;
        void remove_sink(t_log_sink sink, void *user_data) noexcept;
        void set_alloc_logging(bool enable) noexcept;
        bool get_alloc_logging() const noexcept;
        void set_api_logging(bool enable) noexcept;
        bool get_api_logging() const noexcept;
        void close() noexcept;

        void debug(const char *fmt, ...) noexcept;
        void info(const char *fmt, ...) noexcept;
        void warn(const char *fmt, ...) noexcept;
        void error(const char *fmt, ...) noexcept;
};

#endif
