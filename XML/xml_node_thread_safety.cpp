#include <new>

#include "xml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int xml_node_prepare_thread_safety(xml_node *node) noexcept
{
    pt_mutex *mutex_pointer;
    void     *allocated_memory;

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
    allocated_memory = cma_malloc(sizeof(pt_mutex));
    if (!allocated_memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(allocated_memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error_code;

        mutex_error_code = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(allocated_memory);
        ft_errno = mutex_error_code;
        return (-1);
    }
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void xml_node_teardown_thread_safety(xml_node *node) noexcept
{
    if (!node)
        return ;
    if (node->mutex)
    {
        node->mutex->~pt_mutex();
        cma_free(node->mutex);
        node->mutex = ft_nullptr;
    }
    node->thread_safe_enabled = false;
    return ;
}

int xml_node_lock(const xml_node *node, bool *lock_acquired) noexcept
{
    xml_node *mutable_node;
    int       entry_errno;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = ft_errno;
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
    {
        ft_errno = entry_errno;
        return (0);
    }
    mutable_node->mutex->lock(THREAD_ID);
    if (mutable_node->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error_code;

        mutex_error_code = mutable_node->mutex->get_error();
        ft_errno = mutex_error_code;
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

void xml_node_unlock(const xml_node *node, bool lock_acquired) noexcept
{
    xml_node *mutable_node;
    int       entry_errno;

    if (!node || !lock_acquired)
        return ;
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->mutex)
        return ;
    entry_errno = ft_errno;
    mutable_node->mutex->unlock(THREAD_ID);
    if (mutable_node->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error_code;

        mutex_error_code = mutable_node->mutex->get_error();
        ft_errno = mutex_error_code;
        return ;
    }
    ft_errno = entry_errno;
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
