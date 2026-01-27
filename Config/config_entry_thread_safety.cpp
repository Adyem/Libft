#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <new>
#include "../PThread/lock_error_helpers.hpp"

int cnfg_entry_prepare_thread_safety(cnfg_entry *entry)
{
    pt_mutex *mutex_pointer;
    pt_mutex *mutex_pointer;

    if (!entry)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (entry->thread_safe_enabled && entry->mutex)
    {
        return (0);
    }
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    {
        int mutex_error = ft_mutex_pop_last_error(mutex_pointer);

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            delete mutex_pointer;
            ft_global_error_stack_push(mutex_error);
            return (-1);
        }
    }
    entry->mutex = mutex_pointer;
    entry->thread_safe_enabled = true;
    return (0);
}

void cnfg_entry_teardown_thread_safety(cnfg_entry *entry)
{
    if (!entry)
        return ;
    if (entry->mutex)
    {
        entry->mutex->~pt_mutex();
        cma_free(entry->mutex);
        entry->mutex = ft_nullptr;
    }
    entry->thread_safe_enabled = false;
    return ;
}

int cnfg_entry_lock(cnfg_entry *entry, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!entry)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!entry->thread_safe_enabled || !entry->mutex)
    {
        return (0);
    }
    entry->mutex->lock(THREAD_ID);
    {
        int lock_error = ft_mutex_pop_last_error(entry->mutex);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lock_error);
            return (-1);
        }
    }
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void cnfg_entry_unlock(cnfg_entry *entry, bool lock_acquired)
{
    if (!entry || !lock_acquired || !entry->mutex)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry->mutex->unlock(THREAD_ID);
    {
        int unlock_error = ft_mutex_pop_last_error(entry->mutex);

        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return ;
        }
    }
    return ;
}
