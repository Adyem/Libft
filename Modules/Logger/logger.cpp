#include <cstdlib>

#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Template/move.hpp"
#include <new>

ft_logger *g_logger = ft_nullptr;

ft_logger::ft_logger() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _mutex(ft_nullptr),
      _thread_safe_enabled(FT_FALSE), _alloc_logging(FT_FALSE),
      _api_logging(FT_FALSE), _error_code(FT_ERR_SUCCESS)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_logger::~ft_logger() noexcept
{
    (void)this->destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->_thread_safe_enabled = FT_FALSE;
    this->_alloc_logging = FT_FALSE;
    this->_api_logging = FT_FALSE;
    this->_mutex = ft_nullptr;
    return ;
}

int32_t ft_logger::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "ft_logger::initialize",
            "already initialised");
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = FT_FALSE;
    this->_alloc_logging = FT_FALSE;
    this->_api_logging = FT_FALSE;
    this->_error_code = FT_ERR_SUCCESS;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::initialize(const char *path, ft_size_t max_size,
        t_log_level level) noexcept
{
    int32_t operation_result;

    operation_result = this->initialize();
    if (operation_result != FT_ERR_SUCCESS)
        return (operation_result);
    ft_log_set_level(level);
    if (path == ft_nullptr)
        return (FT_ERR_SUCCESS);
    operation_result = this->set_file(path, max_size);
    if (operation_result != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (operation_result);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::initialize(const ft_logger &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "ft_logger::initialize(copy)",
            "source object is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (other._thread_safe_enabled == FT_TRUE)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_error_code);
        }
    }
    this->_alloc_logging = other._alloc_logging;
    this->_api_logging = other._api_logging;
    this->_error_code = other._error_code;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::initialize(ft_logger &&other) noexcept
{
    int32_t operation_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    operation_result = this->initialize(static_cast<const ft_logger &>(other));
    if (operation_result != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (operation_result);
    }
    if (other._mutex != ft_nullptr)
    {
        this->_mutex = other._mutex;
        other._mutex = ft_nullptr;
    }
    other._thread_safe_enabled = FT_FALSE;
    other._alloc_logging = FT_FALSE;
    other._api_logging = FT_FALSE;
    other._error_code = FT_ERR_SUCCESS;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    if (this->_thread_safe_enabled == FT_TRUE)
        (void)this->disable_thread_safety();
    ft_log_close();
    if (g_logger == this)
        g_logger = ft_nullptr;
    this->_alloc_logging = FT_FALSE;
    this->_api_logging = FT_FALSE;
    this->_error_code = FT_ERR_SUCCESS;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t ft_logger::move(ft_logger &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
}

void ft_logger::set_global() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state, "ft_logger::set_global");
    if (this->_initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->set_error(FT_ERR_NOT_INITIALISED);
        return ;
    }
    g_logger = this;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_logger::set_level(t_log_level level) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_level(level);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int32_t ft_logger::set_file(const char *path, ft_size_t max_size) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_set_file(path, max_size);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::set_rotation(ft_size_t max_size, ft_size_t retention_count,
                            uint32_t max_age_seconds) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_set_rotation(max_size, retention_count, max_age_seconds);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::get_rotation(ft_size_t *max_size, ft_size_t *retention_count,
                            uint32_t *max_age_seconds) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_get_rotation(max_size, retention_count, max_age_seconds);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::add_sink(t_log_sink sink, void *user_data) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_add_sink(sink, user_data);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_logger::remove_sink(t_log_sink sink, void *user_data) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_remove_sink(sink, user_data);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::set_alloc_logging(ft_bool enable) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_alloc_logging = enable;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_bool ft_logger::get_alloc_logging() const noexcept
{
    ft_bool lock_acquired;
    ft_bool alloc_logging;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    alloc_logging = this->_alloc_logging;
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (alloc_logging);
}

void ft_logger::set_api_logging(ft_bool enable) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_api_logging = enable;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_bool ft_logger::get_api_logging() const noexcept
{
    ft_bool lock_acquired;
    ft_bool api_logging;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    api_logging = this->_api_logging;
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (api_logging);
}

void ft_logger::set_color(ft_bool enable) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_color(enable);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_bool ft_logger::get_color() const noexcept
{
    ft_bool color_enabled;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);

    color_enabled = ft_log_get_color();
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (color_enabled);
}

void ft_logger::close() noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_close();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int32_t ft_logger::set_syslog(const char *identifier) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_set_syslog(identifier);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::set_remote_sink(const char *host, uint16_t port,
                               ft_bool use_tcp) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_set_remote_sink(host, port, use_tcp);
    if (operation_result != 0)
    {
        int32_t error_code_value;

        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code_value);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_result;

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
    this->_thread_safe_enabled = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::disable_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = FT_FALSE;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int32_t destroy_result = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = FT_FALSE;
    if (destroy_result != FT_ERR_SUCCESS && destroy_result != FT_ERR_INVALID_STATE)
    {
        this->set_error(destroy_result);
        return (destroy_result);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_bool ft_logger::is_thread_safe() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_FALSE);
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

void ft_logger::set_async_queue_limit(ft_size_t limit) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_async_queue_limit(limit);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_size_t ft_logger::get_async_queue_limit() const noexcept
{
    ft_size_t limit;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (0);

    limit = ft_log_get_async_queue_limit();
    const_cast<ft_logger *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (limit);
}

int32_t ft_logger::get_async_metrics(s_log_async_metrics *metrics) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_get_async_metrics(metrics);
    if (operation_result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_logger::reset_async_metrics() noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_reset_async_metrics();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::enable_remote_health(ft_bool enable) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_enable_remote_health(enable);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::set_remote_health_interval(uint32_t interval_seconds) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_set_remote_health_interval(interval_seconds);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int32_t ft_logger::probe_remote_health() noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_probe_remote_health();
    if (operation_result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::get_remote_health(s_log_remote_health *statuses, ft_size_t capacity, ft_size_t *entry_count) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_get_remote_health(statuses, capacity, entry_count);
    if (operation_result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_logger::push_context(const s_log_field *fields, ft_size_t field_count) noexcept
{
    int32_t operation_result;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);

    operation_result = ft_log_context_push(fields, field_count);
    if (operation_result != 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock(lock_acquired);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_logger::pop_context(ft_size_t field_count) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_context_pop(field_count);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

ft_log_context_guard *ft_logger::make_context_guard(const s_log_field *fields,
        ft_size_t field_count) noexcept
{
    ft_log_context_guard *context_guard;
    ft_bool lock_acquired;
    int32_t operation_result;

    context_guard = new (std::nothrow) ft_log_context_guard();
    if (context_guard == ft_nullptr)
        return (ft_nullptr);
    operation_result = context_guard->initialize();
    if (operation_result != FT_ERR_SUCCESS)
    {
        delete context_guard;
        return (ft_nullptr);
    }
    operation_result = context_guard->initialize(fields, field_count);
    if (operation_result != FT_ERR_SUCCESS)
    {
        delete context_guard;
        return (ft_nullptr);
    }

    lock_acquired = FT_FALSE;
    operation_result = this->lock(&lock_acquired);
    if (operation_result != FT_ERR_SUCCESS)
    {
        delete context_guard;
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return (context_guard);
}

int32_t ft_logger::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state, "ft_logger::get_error");
    return (this->_error_code);
}

const char *ft_logger::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state, "ft_logger::get_error_str");
    return (ft_strerror(this->_error_code));
}

void ft_logger::debug(const char *format_string, ...) noexcept
{
    va_list argument_list;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!format_string)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(argument_list, format_string);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_DEBUG, format_string, argument_list);
    else
        ft_log_vwrite(LOG_LEVEL_DEBUG, format_string, argument_list);
    va_end(argument_list);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::info(const char *format_string, ...) noexcept
{
    va_list argument_list;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!format_string)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(argument_list, format_string);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_INFO, format_string, argument_list);
    else
        ft_log_vwrite(LOG_LEVEL_INFO, format_string, argument_list);
    va_end(argument_list);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::warn(const char *format_string, ...) noexcept
{
    va_list argument_list;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!format_string)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(argument_list, format_string);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_WARN, format_string, argument_list);
    else
        ft_log_vwrite(LOG_LEVEL_WARN, format_string, argument_list);
    va_end(argument_list);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::error(const char *format_string, ...) noexcept
{
    va_list argument_list;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;

    if (!format_string)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock(lock_acquired);
        return ;
    }
    va_start(argument_list, format_string);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_ERROR, format_string, argument_list);
    else
        ft_log_vwrite(LOG_LEVEL_ERROR, format_string, argument_list);
    va_end(argument_list);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::structured(t_log_level level, const char *message,
                           const s_log_field *fields,
                           ft_size_t field_count) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    ft_log_structured(level, message, fields, field_count);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

void ft_logger::structured_debug(const char *message,
                                 const s_log_field *fields,
                                 ft_size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_DEBUG, message, fields, field_count);
    return ;
}

void ft_logger::structured_info(const char *message,
                                const s_log_field *fields,
                                ft_size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_INFO, message, fields, field_count);
    return ;
}

void ft_logger::structured_warn(const char *message,
                                const s_log_field *fields,
                                ft_size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_WARN, message, fields, field_count);
    return ;
}

void ft_logger::structured_error(const char *message,
                                 const s_log_field *fields,
                                 ft_size_t field_count) noexcept
{
    this->structured(LOG_LEVEL_ERROR, message, fields, field_count);
    return ;
}

void ft_logger::set_error(int32_t error_code_value) const noexcept
{
    this->_error_code = error_code_value;
    return ;
}

int32_t ft_logger::lock(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_status;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    lock_status = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_status != FT_ERR_SUCCESS)
    {
        this->set_error(lock_status);
        return (lock_status);
    }
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void ft_logger::unlock(ft_bool lock_acquired) const noexcept
{
    if (!lock_acquired)
    {
        return ;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}
