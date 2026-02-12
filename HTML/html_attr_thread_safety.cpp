#include <new>

#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int html_attr_prepare_thread_safety(html_attr *attribute)
{
    pt_mutex *mutex_pointer;

    if (!attribute)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (attribute->thread_safe_enabled && attribute->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    {
        int mutex_error;

        if (mutex_pointer == ft_nullptr)
            mutex_error = FT_ERR_SUCCESS;
        else
            mutex_error = ft_global_error_stack_drop_last_error();

        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            ft_global_error_stack_push(mutex_error);
            return (-1);
        }
    }
    attribute->mutex = mutex_pointer;
    attribute->thread_safe_enabled = true;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

void html_attr_teardown_thread_safety(html_attr *attribute)
{
    if (!attribute)
        return ;
    if (attribute->mutex)
    {
        delete attribute->mutex;
        attribute->mutex = ft_nullptr;
    }
    attribute->thread_safe_enabled = false;
    return ;
}

int html_attr_lock(const html_attr *attribute, bool *lock_acquired)
{
    html_attr *mutable_attribute;

    if (lock_acquired)
        *lock_acquired = false;
    if (!attribute)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->thread_safe_enabled || !mutable_attribute->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    mutable_attribute->mutex->lock(THREAD_ID);
    {
        int lock_error;

        if (mutable_attribute->mutex == ft_nullptr)
            lock_error = FT_ERR_SUCCESS;
        else
            lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(lock_error);
            return (-1);
        }
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

void html_attr_unlock(const html_attr *attribute, bool lock_acquired)
{
    html_attr *mutable_attribute;

    if (!attribute || !lock_acquired)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return ;
    }
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->mutex)
        return ;
    mutable_attribute->mutex->unlock(THREAD_ID);
    {
        int unlock_error;

        if (mutable_attribute->mutex == ft_nullptr)
            unlock_error = FT_ERR_SUCCESS;
        else
            unlock_error = ft_global_error_stack_drop_last_error();

        if (unlock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(unlock_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

bool html_attr_is_thread_safe_enabled(const html_attr *attribute)
{
    if (!attribute)
        return (false);
    if (!attribute->thread_safe_enabled || !attribute->mutex)
        return (false);
    return (true);
}
