#include <new>

#include "xml.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"

int32_t xml_node_prepare_thread_safety(xml_node *node) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error_code;

    if (!node)
        return (FT_ERR_INVALID_ARGUMENT);
    if (node->mutex)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
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

int32_t xml_node_lock(const xml_node *node, ft_bool *lock_acquired) noexcept
{
    xml_node *mutable_node;
    ft_bool has_mutex;
    int32_t mutex_result;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!node)
        return (FT_ERR_INVALID_ARGUMENT);
    mutable_node = const_cast<xml_node *>(node);
    has_mutex = (mutable_node->mutex != ft_nullptr);
    mutex_result = pt_recursive_mutex_lock_if_not_null(mutable_node->mutex);
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired && has_mutex)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void xml_node_unlock(const xml_node *node, ft_bool lock_acquired) noexcept
{
    xml_node *mutable_node;

    if (!node || !lock_acquired)
        return ;
    mutable_node = const_cast<xml_node *>(node);
    (void)pt_recursive_mutex_unlock_if_not_null(mutable_node->mutex);
    return ;
}
