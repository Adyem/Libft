#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t log_sink_prepare_thread_safety(s_log_sink *sink)
{
    pt_mutex *mutex_pointer;
    int32_t      error_code_value;
    int32_t      initialize_result;

    if (!sink)
        return (FT_ERR_INVALID_ARGUMENT);
    if (sink->thread_safe_enabled && sink->mutex)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_NO_MEMORY);
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_result);
    }
    sink->mutex = mutex_pointer;
    sink->thread_safe_enabled = FT_TRUE;
    error_code_value = FT_ERR_SUCCESS;
    return (error_code_value);
}

void log_sink_teardown_thread_safety(s_log_sink *sink)
{
    if (!sink)
        return ;
    if (sink->mutex)
    {
        (void)sink->mutex->destroy();
        delete sink->mutex;
        sink->mutex = ft_nullptr;
    }
    sink->thread_safe_enabled = FT_FALSE;
    return ;
}

int32_t log_sink_lock(const s_log_sink *sink, ft_bool *lock_acquired)
{
    s_log_sink *mutable_sink;
    int32_t error_code_value;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!sink)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_sink = const_cast<s_log_sink *>(sink);
    if (!mutable_sink->thread_safe_enabled || !mutable_sink->mutex)
        return (FT_ERR_SUCCESS);
    error_code_value = pt_mutex_lock_if_not_null(mutable_sink->mutex);
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    error_code_value = FT_ERR_SUCCESS;
    return (error_code_value);
}

void log_sink_unlock(const s_log_sink *sink, ft_bool lock_acquired)
{
    s_log_sink *mutable_sink;

    if (!sink)
        return ;
    if (!lock_acquired)
        return ;
    mutable_sink = const_cast<s_log_sink *>(sink);
    if (!mutable_sink->mutex)
        return ;
    (void)pt_mutex_unlock_if_not_null(mutable_sink->mutex);
    return ;
}
