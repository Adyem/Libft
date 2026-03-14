#include <new>

#include "html_parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"

int32_t html_attr_prepare_thread_safety(html_attr *attribute)
{
    pt_mutex *mutex_pointer;
    int32_t initialize_error;

    if (!attribute)
        return (FT_ERR_INVALID_ARGUMENT);
    if (attribute->thread_safe_enabled && attribute->mutex)
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
    attribute->mutex = mutex_pointer;
    attribute->thread_safe_enabled = FT_TRUE;
    return (FT_ERR_SUCCESS);
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
    attribute->thread_safe_enabled = FT_FALSE;
    return ;
}

int32_t html_attr_lock(const html_attr *attribute, ft_bool *lock_acquired)
{
    html_attr *mutable_attribute;
    int32_t lock_error;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!attribute)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->thread_safe_enabled || !mutable_attribute->mutex)
        return (FT_ERR_SUCCESS);
    lock_error = pt_mutex_lock_if_not_null(mutable_attribute->mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void html_attr_unlock(const html_attr *attribute, ft_bool lock_acquired)
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

ft_bool html_attr_is_thread_safe_enabled(const html_attr *attribute)
{
    if (!attribute)
        return (FT_FALSE);
    if (!attribute->thread_safe_enabled || !attribute->mutex)
        return (FT_FALSE);
    return (FT_TRUE);
}
