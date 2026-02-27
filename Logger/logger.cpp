#include <cstdlib>

#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"

ft_logger *g_logger = ft_nullptr;

ft_logger::ft_logger(const char *path, size_t max_size, t_log_level level) noexcept
    : _mutex(ft_nullptr), _thread_safe_enabled(false), _alloc_logging(false),
      _api_logging(false), _error_code(FT_ERR_SUCCESS)
{
    bool lock_acquired;

    this->set_error(FT_ERR_SUCCESS);
    if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        return ;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_level(level);
    if (path)
    {
        if (ft_log_set_file(path, max_size) != 0)
        {
            int error_code;

            error_code = FT_ERR_SUCCESS;
            if (error_code == FT_ERR_SUCCESS)
                error_code = FT_ERR_INVALID_ARGUMENT;
            this->set_error(error_code);
            this->unlock(lock_acquired);
            return ;
        }
    }
    this->unlock(lock_acquired);
    return ;
}

ft_logger::~ft_logger() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) == FT_ERR_SUCCESS)
    {
        ft_log_close();
        if (g_logger == this)
            g_logger = ft_nullptr;
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
    }
    (void)this->disable_thread_safety();
    return ;
}

void ft_logger::set_global() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (!this->_thread_safe_enabled)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            this->unlock(lock_acquired);
            return ;
        }
    }
    g_logger = this;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::set_level(t_log_level level) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_level(level);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int ft_logger::set_file(const char *path, size_t max_size) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_set_file(path, max_size);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::set_rotation(size_t max_size, size_t retention_count,
                            unsigned int max_age_seconds) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_set_rotation(max_size, retention_count, max_age_seconds);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::get_rotation(size_t *max_size, size_t *retention_count,
                            unsigned int *max_age_seconds) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_get_rotation(max_size, retention_count, max_age_seconds);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::add_sink(t_log_sink sink, void *user_data) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_add_sink(sink, user_data);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

void ft_logger::remove_sink(t_log_sink sink, void *user_data) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_remove_sink(sink, user_data);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::set_alloc_logging(bool enable) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_alloc_logging = enable;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

bool ft_logger::get_alloc_logging() const noexcept
{
    bool lock_acquired;
    bool alloc_logging;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (false);
    alloc_logging = this->_alloc_logging;
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (alloc_logging);
}

void ft_logger::set_api_logging(bool enable) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_api_logging = enable;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

bool ft_logger::get_api_logging() const noexcept
{
    bool lock_acquired;
    bool api_logging;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (false);
    api_logging = this->_api_logging;
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (api_logging);
}

void ft_logger::set_color(bool enable) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_color(enable);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

bool ft_logger::get_color() const noexcept
{
    bool color_enabled;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (false);

    color_enabled = ft_log_get_color();
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (color_enabled);
}

void ft_logger::close() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_close();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int ft_logger::set_syslog(const char *identifier) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_set_syslog(identifier);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::set_remote_sink(const char *host, unsigned short port,
                               bool use_tcp) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_set_remote_sink(host, port, use_tcp);
    if (result != 0)
    {
        int error_code;

        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_result;

    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new(std::nothrow) pt_recursive_mutex();
    if (!mutex_pointer)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_result);
        return (initialize_result);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_logger::disable_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = false;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int destroy_result = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    if (destroy_result != FT_ERR_SUCCESS && destroy_result != FT_ERR_INVALID_STATE)
    {
        this->set_error(destroy_result);
        return (destroy_result);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

bool ft_logger::is_thread_safe() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (false);
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    return (true);
}

void ft_logger::set_async_queue_limit(size_t limit) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_async_queue_limit(limit);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

size_t ft_logger::get_async_queue_limit() const noexcept
{
    size_t limit;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (0);

    limit = ft_log_get_async_queue_limit();
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (limit);
}

int ft_logger::get_async_metrics(s_log_async_metrics *metrics) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_get_async_metrics(metrics);
    if (result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

void ft_logger::reset_async_metrics() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_reset_async_metrics();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::enable_remote_health(bool enable) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_enable_remote_health(enable);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::set_remote_health_interval(unsigned int interval_seconds) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_remote_health_interval(interval_seconds);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int ft_logger::probe_remote_health() noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_probe_remote_health();
    if (result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::get_remote_health(s_log_remote_health *statuses, size_t capacity, size_t *count) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_get_remote_health(statuses, capacity, count);
    if (result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

int ft_logger::push_context(const s_log_field *fields, size_t field_count) noexcept
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (-1);

    result = ft_log_context_push(fields, field_count);
    if (result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (0);
}

void ft_logger::pop_context(size_t field_count) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_context_pop(field_count);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_log_context_guard ft_logger::make_context_guard(const s_log_field *fields,
        size_t field_count) noexcept
{
    ft_log_context_guard context_guard(fields, field_count);
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) == FT_ERR_SUCCESS)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
    }
    return (context_guard);
}

int ft_logger::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_logger::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_logger::debug(const char *fmt, ...) noexcept
{
    va_list args;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_DEBUG, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::info(const char *fmt, ...) noexcept
{
    va_list args;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_INFO, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::warn(const char *fmt, ...) noexcept
{
    va_list args;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_WARN, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::error(const char *fmt, ...) noexcept
{
    va_list args;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_ERROR, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::structured(t_log_level level, const char *message,
                           const s_log_field *fields,
                           size_t field_count) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_structured(level, message, fields, field_count);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::structured_debug(const char *message,
                                 const s_log_field *fields,
                                 size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_DEBUG, message, fields, field_count);
    return ;
}

void ft_logger::structured_info(const char *message,
                                const s_log_field *fields,
                                size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_INFO, message, fields, field_count);
    return ;
}

void ft_logger::structured_warn(const char *message,
                                const s_log_field *fields,
                                size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_WARN, message, fields, field_count);
    return ;
}

void ft_logger::structured_error(const char *message,
                                 const s_log_field *fields,
                                 size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_ERROR, message, fields, field_count);
    return ;
}

void ft_logger::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

int ft_logger::lock(bool *lock_acquired) const noexcept
{
    int lock_result;

    if (lock_acquired)
        *lock_acquired = false;
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void ft_logger::unlock(bool lock_acquired) const noexcept
{
    if (!lock_acquired)
    {
        return ;
    }
    if (pt_recursive_mutex_unlock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        const_cast<ft_logger *>(this)->set_error(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    return ;
}
