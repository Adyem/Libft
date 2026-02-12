#include <new>

#include "parser.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int html_node_prepare_thread_safety(html_node *node)
{
    pt_mutex *mutex_pointer;

    if (!node)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (node->thread_safe_enabled && node->mutex)
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
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

void html_node_teardown_thread_safety(html_node *node)
{
    if (!node)
        return ;
    if (node->mutex)
    {
        delete node->mutex;
        node->mutex = ft_nullptr;
    }
    node->thread_safe_enabled = false;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int html_node_lock(const html_node *node, bool *lock_acquired)
{
    html_node *mutable_node;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    mutable_node->mutex->lock(THREAD_ID);
    {
        int lock_error;

        if (mutable_node->mutex == ft_nullptr)
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

void html_node_unlock(const html_node *node, bool lock_acquired)
{
    html_node *mutable_node;

    if (!node || !lock_acquired)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return ;
    }
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->mutex)
        return ;
    mutable_node->mutex->unlock(THREAD_ID);
    {
        int unlock_error;

        if (mutable_node->mutex == ft_nullptr)
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

bool html_node_is_thread_safe_enabled(const html_node *node)
{
    if (!node)
        return (false);
    if (!node->thread_safe_enabled || !node->mutex)
        return (false);
    return (true);
}
