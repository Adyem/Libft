#include "networking.hpp"
#include "../Errno/errno.hpp"

int socket_config_prepare_thread_safety(SocketConfig *config)
{
    if (config == ft_nullptr)
        return (-1);
    return (0);
}

void socket_config_teardown_thread_safety(SocketConfig *config)
{
    if (config == ft_nullptr)
        return ;
    return ;
}

int socket_config_lock(const SocketConfig *config, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (config == ft_nullptr)
        return (-1);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (0);
}

void socket_config_unlock(const SocketConfig *config, bool lock_acquired)
{
    if (config == ft_nullptr)
        return ;
    if (lock_acquired == false)
        return ;
    return ;
}
