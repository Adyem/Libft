#include "../Basic/environment_lock.hpp"
#include "../CPP_class/class_nullptr.hpp"

#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"

static pt_mutex *g_environment_mutex = ft_nullptr;

static int ft_environment_enable_thread_safety_internal(void)
{
    if (g_environment_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_mutex_create_with_error(&g_environment_mutex));
}

static void ft_environment_disable_thread_safety_internal(void)
{
    pt_mutex_destroy(&g_environment_mutex);
    return ;
}

int ft_environment_lock(void)
{
    if (g_environment_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int lock_result = g_environment_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    ft_global_error_stack_drop_last_error();
    return (0);
}

int ft_environment_unlock(void)
{
    if (g_environment_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int unlock_result = g_environment_mutex->unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    ft_global_error_stack_drop_last_error();
    return (0);
}

int ft_environment_enable_thread_safety(void)
{
    int result = ft_environment_enable_thread_safety_internal();
    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

void ft_environment_disable_thread_safety(void)
{
    ft_environment_disable_thread_safety_internal();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

#endif
