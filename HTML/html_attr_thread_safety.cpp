#include <cstdlib>
#include <new>

#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int html_attr_prepare_thread_safety(html_attr *attribute)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!attribute)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (attribute->thread_safe_enabled && attribute->mutex)
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
    attribute->mutex = mutex_pointer;
    attribute->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void html_attr_teardown_thread_safety(html_attr *attribute)
{
    if (!attribute)
        return ;
    if (attribute->mutex)
    {
        attribute->mutex->~pt_mutex();
        std::free(attribute->mutex);
        attribute->mutex = ft_nullptr;
    }
    attribute->thread_safe_enabled = false;
    return ;
}

int html_attr_lock(const html_attr *attribute, bool *lock_acquired)
{
    html_attr *mutable_attribute;
    int        entry_errno;

    ft_errno = FT_ERR_SUCCESSS;
    if (lock_acquired)
        *lock_acquired = false;
    if (!attribute)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = FT_ERR_SUCCESSS;
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->thread_safe_enabled || !mutable_attribute->mutex)
    {
        ft_errno = entry_errno;
        return (0);
    }
    mutable_attribute->mutex->lock(THREAD_ID);
    if (mutable_attribute->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_attribute->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

void html_attr_unlock(const html_attr *attribute, bool lock_acquired)
{
    html_attr *mutable_attribute;
    int        entry_errno;

    ft_errno = FT_ERR_SUCCESSS;
    if (!attribute || !lock_acquired)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    mutable_attribute = const_cast<html_attr *>(attribute);
    if (!mutable_attribute->mutex)
        return ;
    entry_errno = FT_ERR_SUCCESSS;
    mutable_attribute->mutex->unlock(THREAD_ID);
    if (mutable_attribute->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_attribute->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
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

