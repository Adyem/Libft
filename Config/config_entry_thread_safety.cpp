#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int cnfg_entry_prepare_thread_safety(cnfg_entry *entry)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!entry)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (entry->thread_safe_enabled && entry->mutex)
    {
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        ft_global_error_stack_push(mutex_error);
        return (-1);
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
    if (entry->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(entry->mutex->get_error());
        return (-1);
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
    if (entry->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(entry->mutex->get_error());
        return ;
    }
    return ;
}
