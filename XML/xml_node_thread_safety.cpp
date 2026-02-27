#include <new>

#include "xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"

int xml_node_prepare_thread_safety(xml_node *node) noexcept
{
    pt_mutex *mutex_pointer;
    int mutex_error_code;

    if (!node)
        return (FT_ERR_INVALID_ARGUMENT);
    if (node->mutex)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_NO_MEMORY);
    mutex_error_code = mutex_pointer->initialize();
    if (mutex_error_code != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error_code);
    }
    node->mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
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
    return ;
}

int xml_node_lock(const xml_node *node, bool *lock_acquired) noexcept
{
    xml_node *mutable_node;
    bool has_mutex;
    int mutex_result;

    if (lock_acquired)
        *lock_acquired = false;
    if (!node)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_node = const_cast<xml_node *>(node);
    has_mutex = (mutable_node->mutex != ft_nullptr);
    mutex_result = pt_mutex_lock_if_not_null(mutable_node->mutex);
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired && has_mutex)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void xml_node_unlock(const xml_node *node, bool lock_acquired) noexcept
{
    xml_node *mutable_node;

    if (!node || !lock_acquired)
        return ;
    mutable_node = const_cast<xml_node *>(node);
    (void)pt_mutex_unlock_if_not_null(mutable_node->mutex);
    return ;
}
