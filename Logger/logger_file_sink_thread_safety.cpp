#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int file_sink_prepare_thread_safety(s_file_sink *sink)
{
    pt_mutex *mutex_pointer;
        int      error_code;

    if (!sink)
        return (FT_ERR_INVALID_ARGUMENT);
    if (sink->thread_safe_enabled && sink->mutex)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_NO_MEMORY);
    int initialize_result;

    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_result);
    }
    sink->mutex = mutex_pointer;
    sink->thread_safe_enabled = true;
    error_code = FT_ERR_SUCCESS;
    return (error_code);
}

void file_sink_teardown_thread_safety(s_file_sink *sink)
{
    if (!sink)
        return ;
    if (sink->mutex)
    {
        (void)sink->mutex->destroy();
        delete sink->mutex;
        sink->mutex = ft_nullptr;
    }
    sink->thread_safe_enabled = false;
    return ;
}

int file_sink_lock(const s_file_sink *sink, bool *lock_acquired)
{
    s_file_sink *mutable_sink;
    int error_code;

    if (lock_acquired)
        *lock_acquired = false;
    if (!sink)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_sink = const_cast<s_file_sink *>(sink);
    if (!mutable_sink->thread_safe_enabled || !mutable_sink->mutex)
        return (FT_ERR_SUCCESS);
    error_code = mutable_sink->mutex->lock();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (lock_acquired)
        *lock_acquired = true;
    error_code = FT_ERR_SUCCESS;
    return (error_code);
}

void file_sink_unlock(const s_file_sink *sink, bool lock_acquired)
{
    s_file_sink *mutable_sink;

    if (!sink)
        return ;
    if (!lock_acquired)
        return ;
    mutable_sink = const_cast<s_file_sink *>(sink);
    if (!mutable_sink->mutex)
        return ;
    (void)mutable_sink->mutex->unlock();
    return ;
}
