#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"

static void time_monotonic_point_disable_thread_safety_internal(t_monotonic_time_point *time_point)
{
    if (!time_point)
        return ;
    if (time_point->mutex)
    {
        time_point->mutex->~pt_mutex();
        std::free(time_point->mutex);
        time_point->mutex = ft_nullptr;
    }
    time_point->thread_safe_enabled = FT_FALSE;
    return ;
}

static int32_t  time_monotonic_point_report_result(int32_t error_code)
{
    return (error_code);
}

int32_t time_monotonic_point_enable_thread_safety(t_monotonic_time_point *time_point)
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
        int32_t mutex_error;

        if (mutex_pointer == ft_nullptr)
            mutex_error = FT_ERR_NO_MEMORY;
        else
            mutex_error = mutex_pointer->initialize();

        if (mutex_error != FT_ERR_SUCCESS)
        {
            mutex_pointer->~pt_mutex();
            std::free(memory);
            return (time_monotonic_point_report_result(mutex_error));
        }
    }
    time_point->mutex = mutex_pointer;
    time_point->thread_safe_enabled = FT_TRUE;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
}

void    time_monotonic_point_disable_thread_safety(t_monotonic_time_point *time_point)
{
    if (!time_point)
        return ;
    time_monotonic_point_disable_thread_safety_internal(time_point);
    return ;
}

int32_t time_monotonic_point_lock(const t_monotonic_time_point *time_point, ft_bool *lock_acquired)
{
    t_monotonic_time_point  *mutable_point;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!time_point)
        return (time_monotonic_point_report_result(FT_ERR_INVALID_ARGUMENT));
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->thread_safe_enabled || !mutable_point->mutex)
        return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
    {
        int32_t lock_error;

        lock_error = pt_mutex_lock_if_not_null(mutable_point->mutex);
        if (lock_error != FT_ERR_SUCCESS)
            return (time_monotonic_point_report_result(lock_error));
    }
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESS));
}

void    time_monotonic_point_unlock(const t_monotonic_time_point *time_point, ft_bool lock_acquired)
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
        int32_t unlock_error;

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

ft_bool    time_monotonic_point_is_thread_safe(const t_monotonic_time_point *time_point)
{
    if (!time_point)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (!time_point->thread_safe_enabled || !time_point->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return (FT_FALSE);
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
