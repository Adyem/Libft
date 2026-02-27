#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"

static void time_monotonic_point_disable_thread_safety(t_monotonic_time_point *time_point)
{
    if (!time_point)
        return ;
    if (time_point->mutex)
    {
        time_point->mutex->~pt_mutex();
        std::free(time_point->mutex);
        time_point->mutex = ft_nullptr;
    }
    time_point->thread_safe_enabled = false;
    return ;
}

static int  time_monotonic_point_report_result(int error_code)
{
    return (error_code);
}

int time_monotonic_point_prepare_thread_safety(t_monotonic_time_point *time_point)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!time_point)
        return (time_monotonic_point_report_result(FT_ERR_INVALID_ARGUMENT));
    if (time_point->thread_safe_enabled && time_point->mutex)
        return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
        return (time_monotonic_point_report_result(FT_ERR_NO_MEMORY));
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
            return (time_monotonic_point_report_result(mutex_error));
        }
    }
    time_point->mutex = mutex_pointer;
    time_point->thread_safe_enabled = true;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
}

void    time_monotonic_point_teardown_thread_safety(t_monotonic_time_point *time_point)
{
    if (!time_point)
        return ;
    time_monotonic_point_disable_thread_safety(time_point);
    return ;
}

int time_monotonic_point_lock(const t_monotonic_time_point *time_point, bool *lock_acquired)
{
    t_monotonic_time_point  *mutable_point;

    if (lock_acquired)
        *lock_acquired = false;
    if (!time_point)
        return (time_monotonic_point_report_result(FT_ERR_INVALID_ARGUMENT));
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->thread_safe_enabled || !mutable_point->mutex)
        return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
    {
        int lock_error;

        lock_error = pt_mutex_lock_if_not_null(mutable_point->mutex);
        if (lock_error != FT_ERR_SUCCESS)
            return (time_monotonic_point_report_result(lock_error));
    }
    if (lock_acquired)
        *lock_acquired = true;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
}

void    time_monotonic_point_unlock(const t_monotonic_time_point *time_point, bool lock_acquired)
{
    t_monotonic_time_point  *mutable_point;

    if (!time_point)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!lock_acquired)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->mutex)
    {
        (void)(FT_ERR_INVALID_STATE);
        return ;
    }
    {
        int unlock_error;

        unlock_error = pt_mutex_unlock_if_not_null(mutable_point->mutex);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            (void)(unlock_error);
            return ;
        }
    }
    (void)(FT_ERR_SUCCESS);
    return ;
}

bool    time_monotonic_point_is_thread_safe(const t_monotonic_time_point *time_point)
{
    if (!time_point)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!time_point->thread_safe_enabled || !time_point->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return (false);
    }
    (void)(FT_ERR_SUCCESS);
    return (true);
}
