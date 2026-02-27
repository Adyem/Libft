#include "config.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

int config_entry_prepare_thread_safety(config_entry *entry)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (!entry)
        return (FT_ERR_INVALID_ARGUMENT);
    if (entry->mutex)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    entry->mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void config_entry_teardown_thread_safety(config_entry *entry)
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

int config_entry_lock(config_entry *entry, bool *lock_acquired)
{
    int lock_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!entry)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = pt_mutex_lock_if_not_null(entry->mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired && entry->mutex)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void config_entry_unlock(config_entry *entry, bool lock_acquired)
{
    if (!entry || !lock_acquired)
        return ;
    (void)pt_mutex_unlock_if_not_null(entry->mutex);
    return ;
}
