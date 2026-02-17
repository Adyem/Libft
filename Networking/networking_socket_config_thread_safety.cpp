#include <cstdlib>

#include "networking.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int socket_config_prepare_thread_safety(SocketConfig *config)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (config == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (config->_thread_safe_enabled == true && config->_mutex != ft_nullptr)
    {
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        (void)(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        (void)(mutex_error);
        return (-1);
    }
    config->_mutex = mutex_pointer;
    config->_thread_safe_enabled = true;
    (void)(FT_ERR_SUCCESS);
    return (0);
}

void socket_config_teardown_thread_safety(SocketConfig *config)
{
    if (config == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (config->_mutex != ft_nullptr)
    {
        config->_mutex->~pt_mutex();
        std::free(config->_mutex);
        config->_mutex = ft_nullptr;
    }
    config->_thread_safe_enabled = false;
    (void)(FT_ERR_SUCCESS);
    return ;
}

int socket_config_lock(const SocketConfig *config, bool *lock_acquired)
{
    SocketConfig *mutable_config;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (config == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_config = const_cast<SocketConfig *>(config);
    if (mutable_config->_thread_safe_enabled == false || mutable_config->_mutex == ft_nullptr)
    {
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    mutable_config->_mutex->lock(THREAD_ID);
    if (mutable_config->_mutex->get_error() != FT_ERR_SUCCESS)
    {
        (void)(mutable_config->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    (void)(FT_ERR_SUCCESS);
    return (0);
}

void socket_config_unlock(const SocketConfig *config, bool lock_acquired)
{
    SocketConfig *mutable_config;

    if (config == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (lock_acquired == false)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    mutable_config = const_cast<SocketConfig *>(config);
    if (mutable_config->_mutex == ft_nullptr)
    {
        (void)(FT_ERR_SUCCESS);
        return ;
    }
    mutable_config->_mutex->unlock(THREAD_ID);
    if (mutable_config->_mutex->get_error() != FT_ERR_SUCCESS)
    {
        (void)(mutable_config->_mutex->get_error());
        return ;
    }
    (void)(FT_ERR_SUCCESS);
    return ;
}
