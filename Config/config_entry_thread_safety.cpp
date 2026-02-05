#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <new>

static void cnfg_entry_push_success(void)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

static void cnfg_entry_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

int cnfg_entry_prepare_thread_safety(cnfg_entry *entry)
{
    pt_mutex *mutex_pointer;

    if (!entry)
    {
        cnfg_entry_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (entry->thread_safe_enabled && entry->mutex)
    {
        cnfg_entry_push_success();
        return (0);
    }
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        cnfg_entry_push_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    {
        int mutex_error;

        if (mutex_pointer == ft_nullptr)
            mutex_error = FT_ERR_SUCCESSS;
        else
            mutex_error = ft_global_error_stack_pop_newest();

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            delete mutex_pointer;
            cnfg_entry_push_error(mutex_error);
            return (-1);
        }
    }
    entry->mutex = mutex_pointer;
    entry->thread_safe_enabled = true;
    cnfg_entry_push_success();
    return (0);
}

void cnfg_entry_teardown_thread_safety(cnfg_entry *entry)
{
    if (!entry)
        return ;
    if (entry->mutex)
    {
        delete entry->mutex;
        entry->mutex = ft_nullptr;
    }
    entry->thread_safe_enabled = false;
    cnfg_entry_push_success();
    return ;
}

int cnfg_entry_lock(cnfg_entry *entry, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!entry)
    {
        cnfg_entry_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!entry->thread_safe_enabled || !entry->mutex)
    {
        cnfg_entry_push_success();
        return (0);
    }
    entry->mutex->lock(THREAD_ID);
    {
        int lock_error;

        if (entry->mutex == ft_nullptr)
            lock_error = FT_ERR_SUCCESSS;
        else
            lock_error = ft_global_error_stack_pop_newest();

        if (lock_error != FT_ERR_SUCCESSS)
        {
            cnfg_entry_push_error(lock_error);
            return (-1);
        }
    }
    if (lock_acquired)
        *lock_acquired = true;
    cnfg_entry_push_success();
    return (0);
}

void cnfg_entry_unlock(cnfg_entry *entry, bool lock_acquired)
{
    if (!entry || !lock_acquired || !entry->mutex)
    {
        cnfg_entry_push_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry->mutex->unlock(THREAD_ID);
    {
        int unlock_error;

        if (entry->mutex == ft_nullptr)
            unlock_error = FT_ERR_SUCCESSS;
        else
            unlock_error = ft_global_error_stack_pop_newest();

        if (unlock_error != FT_ERR_SUCCESSS)
        {
            cnfg_entry_push_error(unlock_error);
            return ;
        }
    }
    cnfg_entry_push_success();
    return ;
}
