#include <new>

#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

int html_node_prepare_thread_safety(html_node *node)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (!node)
        return (-1);
    if (node->thread_safe_enabled && node->mutex)
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
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    return (0);
}

void html_node_teardown_thread_safety(html_node *node)
{
    if (!node)
        return ;
    if (node->mutex)
    {
        node->mutex->destroy();
        delete node->mutex;
        node->mutex = ft_nullptr;
    }
    node->thread_safe_enabled = false;
    return ;
}

int html_node_lock(const html_node *node, bool *lock_acquired)
{
    html_node *mutable_node;
    int lock_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
        return (-1);
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
        return (0);
    lock_error = mutable_node->mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void html_node_unlock(const html_node *node, bool lock_acquired)
{
    html_node *mutable_node;

    if (!node || !lock_acquired)
        return ;
    mutable_node = const_cast<html_node *>(node);
    if (!mutable_node->mutex)
        return ;
    mutable_node->mutex->unlock();
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
