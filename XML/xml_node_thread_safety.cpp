#include <new>

#include "xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/lock_error_helpers.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int xml_node_prepare_thread_safety(xml_node *node) noexcept
{
    pt_mutex *mutex_pointer;

    if (!node)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (node->thread_safe_enabled && node->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    int mutex_error_code = ft_mutex_pop_last_error(mutex_pointer);
    if (mutex_error_code != FT_ERR_SUCCESSS)
    {
        delete mutex_pointer;
        ft_global_error_stack_push(mutex_error_code);
        return (-1);
    }
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void xml_node_teardown_thread_safety(xml_node *node) noexcept
{
    if (!node)
        return ;
    if (node->mutex)
    {
        delete node->mutex;
        node->mutex = ft_nullptr;
    }
    node->thread_safe_enabled = false;
    return ;
}

int xml_node_lock(const xml_node *node, bool *lock_acquired) noexcept
{
    xml_node *mutable_node;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    int mutex_result = mutable_node->mutex->lock(THREAD_ID);
    int mutex_error_code = ft_mutex_pop_last_error(mutable_node->mutex);
    {
        int reported_error = mutex_error_code != FT_ERR_SUCCESSS ? mutex_error_code : mutex_result;

        if (reported_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(reported_error);
            return (-1);
        }
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void xml_node_unlock(const xml_node *node, bool lock_acquired) noexcept
{
    xml_node *mutable_node;

    if (!node || !lock_acquired)
    {
        if (!node)
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        else
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->mutex)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    int mutex_result = mutable_node->mutex->unlock(THREAD_ID);
    int mutex_error_code = ft_mutex_pop_last_error(mutable_node->mutex);
    {
        int reported_error = mutex_error_code != FT_ERR_SUCCESSS ? mutex_error_code : mutex_result;

        if (reported_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(reported_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

bool xml_node_is_thread_safe_enabled(const xml_node *node) noexcept
{
    if (!node)
        return (false);
    if (!node->thread_safe_enabled || !node->mutex)
        return (false);
    return (true);
}
