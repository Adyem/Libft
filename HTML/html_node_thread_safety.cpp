#include <cstdlib>
#include <new>

#include "parser.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"

int html_node_prepare_thread_safety(html_node *node)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (node->thread_safe_enabled && node->mutex)
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
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void html_node_teardown_thread_safety(html_node *node)
{
    if (!node)
        return ;
    if (node->mutex)
    {
        node->mutex->~pt_mutex();
        std::free(node->mutex);
        node->mutex = ft_nullptr;
    }
    node->thread_safe_enabled = false;
    return ;
}

int html_node_lock(const html_node *node, bool *lock_acquired)
{
    html_node *mutable_node;

    ft_errno = FT_ERR_SUCCESSS;
    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    mutable_node->mutex->lock(THREAD_ID);
    if (mutable_node->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_node->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void html_node_unlock(const html_node *node, bool lock_acquired)
{
    html_node *mutable_node;

    ft_errno = FT_ERR_SUCCESSS;
    if (!node || !lock_acquired)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->mutex)
        return ;
    mutable_node->mutex->unlock(THREAD_ID);
    if (mutable_node->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = mutable_node->mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
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

