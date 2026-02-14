#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <new>

int cnfg_entry_prepare_thread_safety(cnfg_entry *entry)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (!entry)
        return (-1);
    if (entry->mutex)
        return (0);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (-1);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    entry->mutex = mutex_pointer;
    return (0);
}

void cnfg_entry_teardown_thread_safety(cnfg_entry *entry)
{
    if (!entry)
        return ;
    if (entry->mutex)
    {
        entry->mutex->destroy();
        delete entry->mutex;
        entry->mutex = ft_nullptr;
    }
    return ;
}

int cnfg_entry_lock(cnfg_entry *entry, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!entry)
        return (-1);
    if (!entry->mutex)
        return (0);
    int lock_error = entry->mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void cnfg_entry_unlock(cnfg_entry *entry, bool lock_acquired)
{
    if (!entry || !lock_acquired || !entry->mutex)
        return ;
    entry->mutex->unlock();
    return ;
}
