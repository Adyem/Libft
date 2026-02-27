#include <new>

#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"

int html_attr_prepare_thread_safety(html_attr *attribute)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (!attribute)
        return (FT_ERR_INVALID_ARGUMENT);
    if (attribute->thread_safe_enabled && attribute->mutex)
        return (0);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    attribute->mutex = mutex_pointer;
    attribute->thread_safe_enabled = true;
    return (0);
}

void html_attr_teardown_thread_safety(html_attr *attribute)
{
    if (!attribute)
        return ;
    if (attribute->mutex)
    {
        attribute->mutex->destroy();
        delete attribute->mutex;
        attribute->mutex = ft_nullptr;
    }
    attribute->thread_safe_enabled = false;
    return ;
}

int html_attr_lock(const html_attr *attribute, bool *lock_acquired)
{
    html_attr *mutable_attribute;
    int lock_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!attribute)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->thread_safe_enabled || !mutable_attribute->mutex)
        return (0);
    lock_error = pt_mutex_lock_if_not_null(mutable_attribute->mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void html_attr_unlock(const html_attr *attribute, bool lock_acquired)
{
    html_attr *mutable_attribute;

    if (!attribute || !lock_acquired)
        return ;
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->mutex)
        return ;
    pt_mutex_unlock_if_not_null(mutable_attribute->mutex);
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
