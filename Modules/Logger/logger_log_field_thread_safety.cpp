#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t log_field_prepare_thread_safety(s_log_field *field)
{
    pt_mutex *mutex_pointer;
    int32_t initialize_result;

    if (!field)
    {
        return (FT_ERR_INTERNAL);
    }
    if (field->thread_safe_enabled && field->mutex)
    {
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_INTERNAL);
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (FT_ERR_INTERNAL);
    }
    field->mutex = mutex_pointer;
    field->thread_safe_enabled = FT_TRUE;
    return (FT_ERR_SUCCESS);
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
    field->thread_safe_enabled = FT_FALSE;
    return ;
}

int32_t log_field_lock(const s_log_field *field, ft_bool *lock_acquired)
{
    s_log_field *mutable_field;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!field)
    {
        return (FT_ERR_INTERNAL);
    }
    mutable_field = const_cast<s_log_field *>(field);
    if (!mutable_field->thread_safe_enabled || !mutable_field->mutex)
    {
        return (FT_ERR_SUCCESS);
    }
    if (pt_mutex_lock_if_not_null(mutable_field->mutex) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void log_field_unlock(const s_log_field *field, ft_bool lock_acquired)
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
