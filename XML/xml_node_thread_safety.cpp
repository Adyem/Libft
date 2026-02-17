#include <new>

#include "xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

int xml_node_prepare_thread_safety(xml_node *node) noexcept
{
    pt_mutex *mutex_pointer;
    int mutex_error_code;

    if (!node)
        return (-1);
    if (node->thread_safe_enabled && node->mutex)
        return (0);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (-1);
    mutex_error_code = mutex_pointer->initialize();
    if (mutex_error_code != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    node->mutex = mutex_pointer;
    node->thread_safe_enabled = true;
    return (0);
}

void xml_node_teardown_thread_safety(xml_node *node) noexcept
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

int xml_node_lock(const xml_node *node, bool *lock_acquired) noexcept
{
    xml_node *mutable_node;
    int mutex_result;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
        return (-1);
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->thread_safe_enabled || !mutable_node->mutex)
        return (0);
    mutex_result = mutable_node->mutex->lock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void xml_node_unlock(const xml_node *node, bool lock_acquired) noexcept
{
    xml_node *mutable_node;

    if (!node || !lock_acquired)
        return ;
    mutable_node = const_cast<xml_node *>(node);
    if (!mutable_node->mutex)
        return ;
    (void)mutable_node->mutex->unlock();
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
