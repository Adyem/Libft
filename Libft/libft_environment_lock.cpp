#include "libft_config.hpp"

#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "libft_environment_lock.hpp"
#include "../Errno/errno.hpp"
#include <pthread.h>
#include <mutex>

static pthread_mutex_t g_environment_mutex;
static std::once_flag g_environment_mutex_once;
static int g_environment_mutex_init_error = FT_ER_SUCCESSS;

static void ft_environment_initialize_mutex(void)
{
    int init_result;

    init_result = pthread_mutex_init(&g_environment_mutex, NULL);
    if (init_result != 0)
    {
        g_environment_mutex_init_error = ft_map_system_error(init_result);
    }
    else
    {
        g_environment_mutex_init_error = FT_ER_SUCCESSS;
    }
    return ;
}
static int g_environment_force_lock_failure = 0;
static int g_environment_forced_lock_errno = FT_ER_SUCCESSS;
static int g_environment_force_unlock_failure = 0;
static int g_environment_forced_unlock_errno = FT_ER_SUCCESSS;

int ft_environment_lock(void)
{
    int lock_result;
    int forced_errno;

    std::call_once(g_environment_mutex_once, ft_environment_initialize_mutex);
    if (g_environment_mutex_init_error != FT_ER_SUCCESSS)
    {
        ft_errno = g_environment_mutex_init_error;
        return (-1);
    }
    if (g_environment_force_lock_failure != 0)
    {
        forced_errno = g_environment_forced_lock_errno;
        g_environment_force_lock_failure = 0;
        g_environment_forced_lock_errno = FT_ER_SUCCESSS;
        if (forced_errno != FT_ER_SUCCESSS)
            ft_errno = forced_errno;
        else
            ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_environment_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    return (0);
}

int ft_environment_unlock(void)
{
    int unlock_result;
    int forced_errno;

    std::call_once(g_environment_mutex_once, ft_environment_initialize_mutex);
    if (g_environment_mutex_init_error != FT_ER_SUCCESSS)
    {
        ft_errno = g_environment_mutex_init_error;
        return (-1);
    }
    unlock_result = pthread_mutex_unlock(&g_environment_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    if (g_environment_force_unlock_failure != 0)
    {
        forced_errno = g_environment_forced_unlock_errno;
        g_environment_force_unlock_failure = 0;
        g_environment_forced_unlock_errno = FT_ER_SUCCESSS;
        if (forced_errno != FT_ER_SUCCESSS)
            ft_errno = forced_errno;
        else
            ft_errno = FT_ERR_MUTEX_NOT_OWNER;
        return (-1);
    }
    return (0);
}

void ft_environment_force_lock_failure(int error_code)
{
    g_environment_force_lock_failure = 1;
    g_environment_forced_lock_errno = error_code;
    return ;
}

void ft_environment_force_unlock_failure(int error_code)
{
    g_environment_force_unlock_failure = 1;
    g_environment_forced_unlock_errno = error_code;
    return ;
}

void ft_environment_reset_failures(void)
{
    g_environment_force_lock_failure = 0;
    g_environment_force_unlock_failure = 0;
    g_environment_forced_lock_errno = FT_ER_SUCCESSS;
    g_environment_forced_unlock_errno = FT_ER_SUCCESSS;
    return ;
}
#endif
