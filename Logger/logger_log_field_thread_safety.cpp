#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"

int log_field_prepare_thread_safety(s_log_field *field)
{
    pt_mutex *mutex_pointer;
    int initialize_result;

    if (!field)
    {
        return (-1);
    }
    if (field->thread_safe_enabled && field->mutex)
    {
        return (0);
    }
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (-1);
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    field->mutex = mutex_pointer;
    field->thread_safe_enabled = true;
    return (0);
}

void log_field_teardown_thread_safety(s_log_field *field)
{
    if (!field)
        return ;
    if (field->mutex)
    {
        (void)field->mutex->destroy();
        delete field->mutex;
        field->mutex = ft_nullptr;
    }
    field->thread_safe_enabled = false;
    return ;
}

int log_field_lock(const s_log_field *field, bool *lock_acquired)
{
    s_log_field *mutable_field;

    if (lock_acquired)
        *lock_acquired = false;
    if (!field)
    {
        return (-1);
    }
    mutable_field = const_cast<s_log_field *>(field);
    if (!mutable_field->thread_safe_enabled || !mutable_field->mutex)
    {
        return (0);
    }
    if (pt_mutex_lock_if_not_null(mutable_field->mutex) != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void log_field_unlock(const s_log_field *field, bool lock_acquired)
{
    s_log_field *mutable_field;

    if (!field)
    {
        return ;
    }
    if (!lock_acquired)
        return ;
    mutable_field = const_cast<s_log_field *>(field);
    if (!mutable_field->mutex)
        return ;
    (void)pt_mutex_unlock_if_not_null(mutable_field->mutex);
    return ;
}
