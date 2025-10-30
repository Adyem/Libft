#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
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

int time_duration_ms_prepare_thread_safety(t_duration_milliseconds *duration)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!duration)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (duration->thread_safe_enabled && duration->mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    duration->mutex = mutex_pointer;
    duration->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
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
    int                        entry_errno;

    if (lock_acquired)
        *lock_acquired = false;
    if (!duration)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = ft_errno;
    mutable_duration = const_cast<t_duration_milliseconds *>(duration);
    if (!mutable_duration->thread_safe_enabled || !mutable_duration->mutex)
    {
        ft_errno = entry_errno;
        return (0);
    }
    mutable_duration->mutex->lock(THREAD_ID);
    if (mutable_duration->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_duration->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

void    time_duration_ms_unlock(const t_duration_milliseconds *duration, bool lock_acquired)
{
    t_duration_milliseconds  *mutable_duration;
    int                        entry_errno;

    if (!duration || !lock_acquired)
        return ;
    mutable_duration = const_cast<t_duration_milliseconds *>(duration);
    if (!mutable_duration->mutex)
        return ;
    entry_errno = ft_errno;
    mutable_duration->mutex->unlock(THREAD_ID);
    if (mutable_duration->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_duration->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

bool    time_duration_ms_is_thread_safe_enabled(const t_duration_milliseconds *duration)
{
    if (!duration)
        return (false);
    if (!duration->thread_safe_enabled || !duration->mutex)
        return (false);
    return (true);
}

