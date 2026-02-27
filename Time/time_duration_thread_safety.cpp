#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"

static void time_duration_ms_disable_thread_safety(t_duration_milliseconds *duration)
{
    if (!duration)
        return ;
    if (duration->mutex)
    {
        duration->mutex->~pt_mutex();
        std::free(duration->mutex);
        duration->mutex = ft_nullptr;
    }
    duration->thread_safe_enabled = false;
    return ;
}

static int  time_duration_ms_report_result(int error_code)
{
    return (error_code);
}

int time_duration_ms_prepare_thread_safety(t_duration_milliseconds *duration)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!duration)
        return (time_duration_ms_report_result(FT_ERR_INVALID_ARGUMENT));
    if (duration->thread_safe_enabled && duration->mutex)
        return (time_duration_ms_report_result(FT_ERR_SUCCESS));
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
        return (time_duration_ms_report_result(FT_ERR_NO_MEMORY));
    mutex_pointer = new(memory) pt_mutex();
    {
        int mutex_error;

        if (mutex_pointer == ft_nullptr)
            mutex_error = FT_ERR_SUCCESS;
        else
            mutex_error = FT_ERR_SUCCESS;

        if (mutex_error != FT_ERR_SUCCESS)
        {
            mutex_pointer->~pt_mutex();
            std::free(memory);
            return (time_duration_ms_report_result(mutex_error));
        }
    }
    duration->mutex = mutex_pointer;
    duration->thread_safe_enabled = true;
    return (time_duration_ms_report_result(FT_ERR_SUCCESS));
}

void    time_duration_ms_teardown_thread_safety(t_duration_milliseconds *duration)
{
    if (!duration)
        return ;
    time_duration_ms_disable_thread_safety(duration);
    return ;
}

int time_duration_ms_lock(const t_duration_milliseconds *duration, bool *lock_acquired)
{
    t_duration_milliseconds  *mutable_duration;

    if (lock_acquired)
        *lock_acquired = false;
    if (!duration)
        return (time_duration_ms_report_result(FT_ERR_INVALID_ARGUMENT));
    mutable_duration = const_cast<t_duration_milliseconds *>(duration);
    if (!mutable_duration->thread_safe_enabled || !mutable_duration->mutex)
        return (time_duration_ms_report_result(FT_ERR_SUCCESS));
    {
        int lock_error;

        lock_error = pt_mutex_lock_if_not_null(mutable_duration->mutex);
        if (lock_error != FT_ERR_SUCCESS)
            return (time_duration_ms_report_result(lock_error));
    }
    if (lock_acquired)
        *lock_acquired = true;
    return (time_duration_ms_report_result(FT_ERR_SUCCESS));
}

void    time_duration_ms_unlock(const t_duration_milliseconds *duration, bool lock_acquired)
{
    t_duration_milliseconds  *mutable_duration;

    if (!duration)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!lock_acquired)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    mutable_duration = const_cast<t_duration_milliseconds *>(duration);
    if (!mutable_duration->mutex)
    {
        (void)(FT_ERR_INVALID_STATE);
        return ;
    }
    {
        int unlock_error;

        unlock_error = pt_mutex_unlock_if_not_null(mutable_duration->mutex);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            (void)(unlock_error);
            return ;
        }
    }
    (void)(FT_ERR_SUCCESS);
    return ;
}

bool    time_duration_ms_is_thread_safe(const t_duration_milliseconds *duration)
{
    if (!duration)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!duration->thread_safe_enabled || !duration->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return (false);
    }
    (void)(FT_ERR_SUCCESS);
    return (true);
}
