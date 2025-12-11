#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int log_field_prepare_thread_safety(s_log_field *field)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!field)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (field->thread_safe_enabled && field->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    field->mutex = mutex_pointer;
    field->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void log_field_teardown_thread_safety(s_log_field *field)
{
    if (!field)
        return ;
    if (field->mutex)
    {
        field->mutex->~pt_mutex();
        std::free(field->mutex);
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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    mutable_field = const_cast<s_log_field *>(field);
    if (!mutable_field->thread_safe_enabled || !mutable_field->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    mutable_field->mutex->lock(THREAD_ID);
    if (mutable_field->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_field->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void log_field_unlock(const s_log_field *field, bool lock_acquired)
{
    s_log_field *mutable_field;

    if (!field)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (!lock_acquired)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    mutable_field = const_cast<s_log_field *>(field);
    if (!mutable_field->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    mutable_field->mutex->unlock(THREAD_ID);
    if (mutable_field->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_field->mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
