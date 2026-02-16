#include <new>

#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

int html_attr_prepare_thread_safety(html_attr *attribute)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (!attribute)
        return (-1);
    if (attribute->thread_safe_enabled && attribute->mutex)
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
        return (-1);
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->thread_safe_enabled || !mutable_attribute->mutex)
        return (0);
    lock_error = mutable_attribute->mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
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
    mutable_attribute->mutex->unlock();
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
