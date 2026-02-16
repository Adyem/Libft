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
    int         error_code;

    if (!time_info)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (time_info->thread_safe_enabled && time_info->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        (void)(FT_ERR_NO_MEMORY);
        return (-1);
    }
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
            (void)(mutex_error);
            return (-1);
        }
    }
    time_info->mutex = mutex_pointer;
    time_info->thread_safe_enabled = true;
    error_code = FT_ERR_SUCCESS;
    (void)(error_code);
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

    if (lock_acquired)
        *lock_acquired = false;
    if (!time_info)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_info = const_cast<t_time_info *>(time_info);
    if (!mutable_info->thread_safe_enabled || !mutable_info->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    {
        int lock_error;

        lock_error = mutable_info->mutex->lock();
        if (lock_error != FT_ERR_SUCCESS)
        {
            (void)(lock_error);
            return (-1);
        }
    }
    if (lock_acquired)
        *lock_acquired = true;
    (void)(FT_ERR_SUCCESS);
    return (0);
}

void    time_info_unlock(const t_time_info *time_info, bool lock_acquired)
{
    t_time_info *mutable_info;

    if (!time_info)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!lock_acquired)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    mutable_info = const_cast<t_time_info *>(time_info);
    if (!mutable_info->mutex)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    {
        int unlock_error;

        unlock_error = mutable_info->mutex->unlock();
        if (unlock_error != FT_ERR_SUCCESS)
        {
            (void)(unlock_error);
            return ;
        }
    }
    (void)(FT_ERR_SUCCESS);
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
