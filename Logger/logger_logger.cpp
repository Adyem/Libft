#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"

ft_logger *g_logger = ft_nullptr;

ft_logger::ft_logger(const char *path, size_t max_size, t_log_level level) noexcept
    : _alloc_logging(false), _api_logging(false), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    ft_log_set_level(level);
    if (path)
    {
        if (ft_log_set_file(path, max_size) != 0)
        {
            int error_code;

            error_code = ft_errno;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_INVALID_ARGUMENT;
            this->set_error(error_code);
            return ;
        }
    }
    return ;
}

ft_logger::~ft_logger() noexcept
{
    ft_log_close();
    if (g_logger == this)
        g_logger = ft_nullptr;
    this->set_error(ft_errno);
    return ;
}

void ft_logger::set_global() noexcept
{
    g_logger = this;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_logger::set_level(t_log_level level) noexcept
{
    ft_log_set_level(level);
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_logger::set_file(const char *path, size_t max_size) noexcept
{
    int result;

    result = ft_log_set_file(path, max_size);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::set_rotation(size_t max_size, size_t retention_count,
                            unsigned int max_age_seconds) noexcept
{
    int result;

    result = ft_log_set_rotation(max_size, retention_count, max_age_seconds);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::get_rotation(size_t *max_size, size_t *retention_count,
                            unsigned int *max_age_seconds) noexcept
{
    int result;

    result = ft_log_get_rotation(max_size, retention_count, max_age_seconds);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::add_sink(t_log_sink sink, void *user_data) noexcept
{
    int result;

    result = ft_log_add_sink(sink, user_data);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_logger::remove_sink(t_log_sink sink, void *user_data) noexcept
{
    ft_log_remove_sink(sink, user_data);
    this->set_error(ft_errno);
    return ;
}

void ft_logger::set_alloc_logging(bool enable) noexcept
{
    this->_alloc_logging = enable;
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_logger::get_alloc_logging() const noexcept
{
    const_cast<ft_logger *>(this)->set_error(ER_SUCCESS);
    return (this->_alloc_logging);
}

void ft_logger::set_api_logging(bool enable) noexcept
{
    this->_api_logging = enable;
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_logger::get_api_logging() const noexcept
{
    const_cast<ft_logger *>(this)->set_error(ER_SUCCESS);
    return (this->_api_logging);
}

void ft_logger::set_color(bool enable) noexcept
{
    ft_log_set_color(enable);
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_logger::get_color() const noexcept
{
    bool color_enabled;

    color_enabled = ft_log_get_color();
    const_cast<ft_logger *>(this)->set_error(ER_SUCCESS);
    return (color_enabled);
}

void ft_logger::close() noexcept
{
    ft_log_close();
    this->set_error(ft_errno);
    return ;
}

int ft_logger::set_syslog(const char *identifier) noexcept
{
    int result;

    result = ft_log_set_syslog(identifier);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::set_remote_sink(const char *host, unsigned short port,
                               bool use_tcp) noexcept
{
    int result;

    result = ft_log_set_remote_sink(host, port, use_tcp);
    if (result != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_logger::set_async_queue_limit(size_t limit) noexcept
{
    ft_log_set_async_queue_limit(limit);
    this->set_error(ft_errno);
    return ;
}

size_t ft_logger::get_async_queue_limit() const noexcept
{
    size_t limit;

    limit = ft_log_get_async_queue_limit();
    const_cast<ft_logger *>(this)->set_error(ft_errno);
    return (limit);
}

int ft_logger::get_async_metrics(s_log_async_metrics *metrics) noexcept
{
    int result;

    result = ft_log_get_async_metrics(metrics);
    if (result != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_logger::reset_async_metrics() noexcept
{
    ft_log_reset_async_metrics();
    this->set_error(ft_errno);
    return ;
}

void ft_logger::enable_remote_health(bool enable) noexcept
{
    ft_log_enable_remote_health(enable);
    this->set_error(ft_errno);
    return ;
}

void ft_logger::set_remote_health_interval(unsigned int interval_seconds) noexcept
{
    ft_log_set_remote_health_interval(interval_seconds);
    this->set_error(ft_errno);
    return ;
}

int ft_logger::probe_remote_health() noexcept
{
    int result;

    result = ft_log_probe_remote_health();
    if (result != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::get_remote_health(s_log_remote_health *statuses, size_t capacity, size_t *count) noexcept
{
    int result;

    result = ft_log_get_remote_health(statuses, capacity, count);
    if (result != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int ft_logger::push_context(const s_log_field *fields, size_t field_count) noexcept
{
    int result;

    result = ft_log_context_push(fields, field_count);
    if (result != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_logger::pop_context(size_t field_count) noexcept
{
    ft_log_context_pop(field_count);
    this->set_error(ft_errno);
    return ;
}

ft_log_context_guard ft_logger::make_context_guard(const s_log_field *fields,
        size_t field_count) noexcept
{
    ft_log_context_guard guard(fields, field_count);

    this->set_error(guard.get_error());
    return (guard);
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

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_errno = ER_SUCCESS;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_DEBUG, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
    if (ft_errno != ER_SUCCESS)
        this->set_error(ft_errno);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_logger::info(const char *fmt, ...) noexcept
{
    va_list args;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_errno = ER_SUCCESS;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_INFO, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
    if (ft_errno != ER_SUCCESS)
        this->set_error(ft_errno);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_logger::warn(const char *fmt, ...) noexcept
{
    va_list args;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_errno = ER_SUCCESS;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_WARN, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
    if (ft_errno != ER_SUCCESS)
        this->set_error(ft_errno);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_logger::error(const char *fmt, ...) noexcept
{
    va_list args;

    if (!fmt)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_errno = ER_SUCCESS;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_ERROR, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
    if (ft_errno != ER_SUCCESS)
        this->set_error(ft_errno);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_logger::structured(t_log_level level, const char *message,
                           const s_log_field *fields,
                           size_t field_count) noexcept
{
    ft_log_structured(level, message, fields, field_count);
    this->set_error(ft_errno);
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
    ft_errno = error_code;
    return ;
}
