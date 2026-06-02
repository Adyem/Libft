#include "networking.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t socket_config_prepare_thread_safety(SocketConfig *config)
{
    if (config == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_SUCCESS);
}

void socket_config_teardown_thread_safety(SocketConfig *config)
{
    if (config == ft_nullptr)
        return ;
    return ;
}

int32_t socket_config_lock(const SocketConfig *config, ft_bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (config == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void socket_config_unlock(const SocketConfig *config, ft_bool lock_acquired)
{
    if (config == ft_nullptr)
        return ;
    if (lock_acquired == FT_FALSE)
        return ;
    return ;
}
