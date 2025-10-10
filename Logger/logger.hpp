#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <cstddef>
#include <cstdarg>

enum t_log_level {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
};



typedef void (*t_log_sink)(const char *message, void *user_data);
int     ft_log_add_sink(t_log_sink sink, void *user_data);
void    ft_log_remove_sink(t_log_sink sink, void *user_data);

struct s_log_field
{
    const char *key;
    const char *value;
};

struct s_log_async_metrics
{
    size_t pending_messages;
    size_t peak_pending_messages;
    size_t dropped_messages;
};

void    ft_log_set_level(t_log_level level);
int     ft_log_set_file(const char *path, size_t max_size);
int     ft_log_set_rotation(size_t max_size, size_t retention_count,
                            unsigned int max_age_seconds);
int     ft_log_get_rotation(size_t *max_size, size_t *retention_count,
                            unsigned int *max_age_seconds);
void    ft_log_close();
void    ft_log_set_alloc_logging(bool enable);
bool    ft_log_get_alloc_logging();
void    ft_log_set_api_logging(bool enable);
bool    ft_log_get_api_logging();
void    ft_log_set_color(bool enable);
bool    ft_log_get_color();
void    ft_json_sink(const char *message, void *user_data);
void    ft_syslog_sink(const char *message, void *user_data);
int     ft_log_set_syslog(const char *identifier);
int     ft_log_set_remote_sink(const char *host, unsigned short port,
                               bool use_tcp);

void ft_log_debug(const char *fmt, ...);
void ft_log_info(const char *fmt, ...);
void ft_log_warn(const char *fmt, ...);
void ft_log_error(const char *fmt, ...);
void ft_log_enable_async(bool enable);
void ft_log_enqueue(t_log_level level, const char *fmt, va_list args);
void ft_log_structured(t_log_level level, const char *message,
                       const s_log_field *fields, size_t field_count);
void ft_log_set_async_queue_limit(size_t limit);
size_t ft_log_get_async_queue_limit();
int  ft_log_get_async_metrics(s_log_async_metrics *metrics);
void ft_log_reset_async_metrics();
void ft_log_debug_structured(const char *message, const s_log_field *fields,
                             size_t field_count);
void ft_log_info_structured(const char *message, const s_log_field *fields,
                            size_t field_count);
void ft_log_warn_structured(const char *message, const s_log_field *fields,
                            size_t field_count);
void ft_log_error_structured(const char *message, const s_log_field *fields,
                             size_t field_count);

class ft_logger
{
    private:
        bool _alloc_logging;
        bool _api_logging;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_logger(const char *path = nullptr, size_t max_size = 0,
                  t_log_level level = LOG_LEVEL_INFO) noexcept;
        ~ft_logger() noexcept;

        ft_logger(const ft_logger&) = delete;
        ft_logger &operator=(const ft_logger&) = delete;

        void set_global() noexcept;
        void set_level(t_log_level level) noexcept;
        int  set_file(const char *path, size_t max_size) noexcept;
        int  set_rotation(size_t max_size, size_t retention_count,
                          unsigned int max_age_seconds) noexcept;
        int  get_rotation(size_t *max_size, size_t *retention_count,
                          unsigned int *max_age_seconds) noexcept;
        int  add_sink(t_log_sink sink, void *user_data) noexcept;
        void remove_sink(t_log_sink sink, void *user_data) noexcept;
        void set_alloc_logging(bool enable) noexcept;
        bool get_alloc_logging() const noexcept;
        void set_api_logging(bool enable) noexcept;
        bool get_api_logging() const noexcept;
        void set_color(bool enable) noexcept;
        bool get_color() const noexcept;
        void close() noexcept;
        int  set_syslog(const char *identifier) noexcept;
        int  set_remote_sink(const char *host, unsigned short port,
                             bool use_tcp) noexcept;
        void set_async_queue_limit(size_t limit) noexcept;
        size_t get_async_queue_limit() const noexcept;
        int  get_async_metrics(s_log_async_metrics *metrics) noexcept;
        void reset_async_metrics() noexcept;
        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;

        void debug(const char *fmt, ...) noexcept;
        void info(const char *fmt, ...) noexcept;
        void warn(const char *fmt, ...) noexcept;
        void error(const char *fmt, ...) noexcept;
        void structured(t_log_level level, const char *message,
                        const s_log_field *fields,
                        size_t field_count) noexcept;
        void structured_debug(const char *message, const s_log_field *fields,
                              size_t field_count) noexcept;
        void structured_info(const char *message, const s_log_field *fields,
                             size_t field_count) noexcept;
        void structured_warn(const char *message, const s_log_field *fields,
                             size_t field_count) noexcept;
        void structured_error(const char *message, const s_log_field *fields,
                              size_t field_count) noexcept;
};

#endif
