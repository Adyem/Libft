#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <cstddef>
#include <cstdarg>
#include "../Time/time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread.hpp"

class pt_recursive_mutex;

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

struct s_log_remote_health
{
    const char  *host;
    unsigned short port;
    bool use_tcp;
    bool reachable;
    t_time last_check;
    int  last_error;
};

struct s_log_field
{
    pt_mutex   *mutex;
    bool        thread_safe_enabled;
    const char *key;
    const char *value;

    s_log_field()
        : mutex(ft_nullptr), thread_safe_enabled(false), key(ft_nullptr), value(ft_nullptr)
    {
        return ;
    }
};

struct s_log_async_metrics
{
    pt_mutex *mutex;
    bool      thread_safe_enabled;
    size_t pending_messages;
    size_t peak_pending_messages;
    size_t dropped_messages;

    s_log_async_metrics()
        : mutex(ft_nullptr), thread_safe_enabled(false), pending_messages(0),
          peak_pending_messages(0), dropped_messages(0)
    {
        return ;
    }
};

int     log_field_prepare_thread_safety(s_log_field *field);
void    log_field_teardown_thread_safety(s_log_field *field);
int     log_field_lock(const s_log_field *field, bool *lock_acquired);
void    log_field_unlock(const s_log_field *field, bool lock_acquired);

int     log_async_metrics_prepare_thread_safety(s_log_async_metrics *metrics);
void    log_async_metrics_teardown_thread_safety(s_log_async_metrics *metrics);
int     log_async_metrics_lock(s_log_async_metrics *metrics, bool *lock_acquired);
void    log_async_metrics_unlock(s_log_async_metrics *metrics, bool lock_acquired);

struct s_log_lock_contention_sample
{
    pthread_mutex_t   *mutex_pointer;
    pt_thread_id_type owner_thread;
    pt_thread_id_type waiting_thread;
    long              wait_duration_ms;
    bool              priority_inversion;

    s_log_lock_contention_sample()
        : mutex_pointer(ft_nullptr), owner_thread(0), waiting_thread(0),
          wait_duration_ms(0), priority_inversion(false)
    {
        return ;
    }
};

struct s_log_lock_contention_statistics
{
    size_t total_samples;
    size_t priority_inversions;
    size_t skipped_samples;
    long longest_wait_ms;
    double average_wait_ms;

    s_log_lock_contention_statistics()
        : total_samples(0), priority_inversions(0), skipped_samples(0),
          longest_wait_ms(0), average_wait_ms(0.0)
    {
        return ;
    }
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
int     ft_log_add_redaction(const char *pattern);
int     ft_log_add_redaction_with_replacement(const char *pattern,
                               const char *replacement);
void    ft_log_clear_redactions();
void    ft_log_enable_remote_health(bool enable);
void    ft_log_set_remote_health_interval(unsigned int interval_seconds);
int     ft_log_probe_remote_health();
int     ft_log_get_remote_health(s_log_remote_health *statuses,
            size_t capacity, size_t *count);
int     ft_log_context_push(const s_log_field *fields, size_t field_count);
void    ft_log_context_pop(size_t field_count);
void    ft_log_context_clear();

class ft_log_context_guard
{
    private:
        size_t _pushed_count;
        bool _active;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        ft_log_context_guard() noexcept;
        ft_log_context_guard(const s_log_field *fields, size_t field_count) noexcept;
        ~ft_log_context_guard() noexcept;

        ft_log_context_guard(const ft_log_context_guard &) = delete;
        ft_log_context_guard &operator=(const ft_log_context_guard &) = delete;

        ft_log_context_guard(ft_log_context_guard &&other) noexcept;
        ft_log_context_guard &operator=(ft_log_context_guard &&other) noexcept;

        void release() noexcept;
        bool is_active() const noexcept;
        int get_error() const;
        const char *get_error_str() const;
};

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
void ft_log_enable_lock_contention_sampling(bool enable);
void ft_log_set_lock_contention_sampling_interval(unsigned int interval_ms);
void ft_log_set_lock_contention_priority_threshold(long threshold_ms);
int  ft_log_sample_lock_contention(s_log_lock_contention_sample *samples, size_t capacity, size_t *count);
int  ft_log_lock_contention_get_statistics(s_log_lock_contention_statistics *statistics);
void ft_log_lock_contention_reset_statistics();

class ft_logger
{
    private:
        pt_recursive_mutex *_mutex;
        bool _thread_safe_enabled;
        bool _alloc_logging;
        bool _api_logging;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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
        int  enable_thread_safety() noexcept;
        int  disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        void set_async_queue_limit(size_t limit) noexcept;
        size_t get_async_queue_limit() const noexcept;
        int  get_async_metrics(s_log_async_metrics *metrics) noexcept;
        void reset_async_metrics() noexcept;
        void enable_remote_health(bool enable) noexcept;
        void set_remote_health_interval(unsigned int interval_seconds) noexcept;
        int  probe_remote_health() noexcept;
        int  get_remote_health(s_log_remote_health *statuses,
                               size_t capacity, size_t *count) noexcept;
        int  push_context(const s_log_field *fields, size_t field_count) noexcept;
        void pop_context(size_t field_count) noexcept;
        ft_log_context_guard make_context_guard(const s_log_field *fields,
                size_t field_count) noexcept;
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
