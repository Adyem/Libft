#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"

static void cnfg_entry_push_success(void)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

static void cnfg_entry_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

int cnfg_entry_prepare_thread_safety(cnfg_entry *entry)
{
    if (!entry)
    {
        cnfg_entry_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (entry->mutex)
    {
        cnfg_entry_push_success();
        return (0);
    }
    int mutex_error = pt_mutex_create_with_error(&entry->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        cnfg_entry_push_error(mutex_error);
        return (-1);
    }
    cnfg_entry_push_success();
    return (0);
}

void cnfg_entry_teardown_thread_safety(cnfg_entry *entry)
{
    if (!entry)
        return ;
    pt_mutex_destroy(&entry->mutex);
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
    if (!entry->mutex)
    {
        cnfg_entry_push_success();
        return (0);
    }
    int lock_error = pt_mutex_lock_if_valid(entry->mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void cnfg_entry_unlock(cnfg_entry *entry, bool lock_acquired)
{
    if (!entry || !lock_acquired || !entry->mutex)
    {
        cnfg_entry_push_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    pt_mutex_unlock_if_valid(entry->mutex);
    return ;
}
