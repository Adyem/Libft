#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
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

static int  time_monotonic_point_report_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int time_monotonic_point_prepare_thread_safety(t_monotonic_time_point *time_point)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!time_point)
        return (time_monotonic_point_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    if (time_point->thread_safe_enabled && time_point->mutex)
        return (time_monotonic_point_report_result(FT_ERR_SUCCESSS, 0));
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
        return (time_monotonic_point_report_result(FT_ERR_NO_MEMORY, -1));
    mutex_pointer = new(memory) pt_mutex();
    {
        int mutex_error;

        if (mutex_pointer == ft_nullptr)
            mutex_error = FT_ERR_SUCCESSS;
        else
            mutex_error = ft_global_error_stack_drop_last_error();

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            mutex_pointer->~pt_mutex();
            std::free(memory);
            return (time_monotonic_point_report_result(mutex_error, -1));
        }
    }
    time_point->mutex = mutex_pointer;
    time_point->thread_safe_enabled = true;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESSS, 0));
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
        return (time_monotonic_point_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->thread_safe_enabled || !mutable_point->mutex)
        return (time_monotonic_point_report_result(FT_ERR_SUCCESSS, 0));
    mutable_point->mutex->lock(THREAD_ID);
    {
        int lock_error;

        if (mutable_point->mutex == ft_nullptr)
            lock_error = FT_ERR_SUCCESSS;
        else
            lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESSS)
            return (time_monotonic_point_report_result(lock_error, -1));
    }
    if (lock_acquired)
        *lock_acquired = true;
    return (time_monotonic_point_report_result(FT_ERR_SUCCESSS, 0));
}

void    time_monotonic_point_unlock(const t_monotonic_time_point *time_point, bool lock_acquired)
{
    t_monotonic_time_point  *mutable_point;

    if (!time_point)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!lock_acquired)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->mutex)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    mutable_point->mutex->unlock(THREAD_ID);
    {
        int unlock_error;

        if (mutable_point->mutex == ft_nullptr)
            unlock_error = FT_ERR_SUCCESSS;
        else
            unlock_error = ft_global_error_stack_drop_last_error();

        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

bool    time_monotonic_point_is_thread_safe_enabled(const t_monotonic_time_point *time_point)
{
    if (!time_point)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!time_point->thread_safe_enabled || !time_point->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}
