#include <cstdlib>
#include <new>

#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

static void time_info_disable_thread_safety(t_time_info *time_info)
{
    if (!time_info)
        return ;
    if (time_info->mutex)
    {
        time_info->mutex->~pt_mutex();
        std::free(time_info->mutex);
        time_info->mutex = ft_nullptr;
    }
    time_info->thread_safe_enabled = false;
    return ;
}

int time_info_prepare_thread_safety(t_time_info *time_info)
{
    pt_mutex    *mutex_pointer;
    void        *memory;

    if (!time_info)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (time_info->thread_safe_enabled && time_info->mutex)
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
    time_info->mutex = mutex_pointer;
    time_info->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void    time_info_teardown_thread_safety(t_time_info *time_info)
{
    if (!time_info)
        return ;
    time_info_disable_thread_safety(time_info);
    return ;
}

int time_info_lock(const t_time_info *time_info, bool *lock_acquired)
{
    t_time_info *mutable_info;
    int          entry_errno;

    if (lock_acquired)
        *lock_acquired = false;
    if (!time_info)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = ft_errno;
    mutable_info = const_cast<t_time_info *>(time_info);
    if (!mutable_info->thread_safe_enabled || !mutable_info->mutex)
    {
        ft_errno = entry_errno;
        return (0);
    }
    mutable_info->mutex->lock(THREAD_ID);
    if (mutable_info->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_info->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

void    time_info_unlock(const t_time_info *time_info, bool lock_acquired)
{
    t_time_info *mutable_info;
    int          entry_errno;

    if (!time_info || !lock_acquired)
        return ;
    mutable_info = const_cast<t_time_info *>(time_info);
    if (!mutable_info->mutex)
        return ;
    entry_errno = ft_errno;
    mutable_info->mutex->unlock(THREAD_ID);
    if (mutable_info->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_info->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

bool    time_info_is_thread_safe_enabled(const t_time_info *time_info)
{
    if (!time_info)
        return (false);
    if (!time_info->thread_safe_enabled || !time_info->mutex)
        return (false);
    return (true);
}

