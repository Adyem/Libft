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

int time_monotonic_point_prepare_thread_safety(t_monotonic_time_point *time_point)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!time_point)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (time_point->thread_safe_enabled && time_point->mutex)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ER_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    time_point->mutex = mutex_pointer;
    time_point->thread_safe_enabled = true;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
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
    int                      entry_errno;

    if (lock_acquired)
        *lock_acquired = false;
    if (!time_point)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = ft_errno;
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->thread_safe_enabled || !mutable_point->mutex)
    {
        ft_errno = entry_errno;
        return (0);
    }
    mutable_point->mutex->lock(THREAD_ID);
    if (mutable_point->mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = mutable_point->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

void    time_monotonic_point_unlock(const t_monotonic_time_point *time_point, bool lock_acquired)
{
    t_monotonic_time_point  *mutable_point;
    int                      entry_errno;

    if (!time_point || !lock_acquired)
        return ;
    mutable_point = const_cast<t_monotonic_time_point *>(time_point);
    if (!mutable_point->mutex)
        return ;
    entry_errno = ft_errno;
    mutable_point->mutex->unlock(THREAD_ID);
    if (mutable_point->mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = mutable_point->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

bool    time_monotonic_point_is_thread_safe_enabled(const t_monotonic_time_point *time_point)
{
    if (!time_point)
        return (false);
    if (!time_point->thread_safe_enabled || !time_point->mutex)
        return (false);
    return (true);
}

