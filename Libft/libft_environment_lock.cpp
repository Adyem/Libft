#include "libft_environment_lock.hpp"
#include "../CPP_class/class_nullptr.hpp"

#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"

static pt_mutex *g_environment_mutex = ft_nullptr;
static bool g_environment_auto_initialize = true;
static int g_environment_force_lock_failure = 0;
static int g_environment_forced_lock_errno = FT_ERR_SUCCESSS;
static int g_environment_force_unlock_failure = 0;
static int g_environment_forced_unlock_errno = FT_ERR_SUCCESSS;

static int ft_environment_enable_thread_safety_internal(void);
static void ft_environment_disable_thread_safety_internal(void);
static int ft_environment_maybe_auto_initialize(void);
static int ft_environment_lock_mutex(void);
static int ft_environment_unlock_mutex(void);

static int ft_environment_enable_thread_safety_internal(void)
{
    if (g_environment_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_mutex_create_with_error(&g_environment_mutex));
}

static void ft_environment_disable_thread_safety_internal(void)
{
    pt_mutex_destroy(&g_environment_mutex);
    return ;
}

static int ft_environment_maybe_auto_initialize(void)
{
    if (!g_environment_auto_initialize)
        return (FT_ERR_SUCCESSS);
    if (g_environment_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (ft_environment_enable_thread_safety_internal());
}

static int ft_environment_lock_mutex(void)
{
    return (pt_mutex_lock_if_valid(g_environment_mutex));
}

static int ft_environment_unlock_mutex(void)
{
    return (pt_mutex_unlock_if_valid(g_environment_mutex));
}

int ft_environment_lock(void)
{
    int auto_init_error;
    int lock_result;
    int forced_errno;
    int error_code;

    auto_init_error = ft_environment_maybe_auto_initialize();
    if (auto_init_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(auto_init_error);
        return (-1);
    }
    if (g_environment_force_lock_failure != 0)
    {
        forced_errno = g_environment_forced_lock_errno;
        g_environment_force_lock_failure = 0;
        g_environment_forced_lock_errno = FT_ERR_SUCCESSS;
        if (forced_errno != FT_ERR_SUCCESSS)
            error_code = forced_errno;
        else
            error_code = FT_ERR_MUTEX_ALREADY_LOCKED;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    lock_result = ft_environment_lock_mutex();
    if (lock_result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

int ft_environment_unlock(void)
{
    int auto_init_error;
    int unlock_result;
    int forced_errno;
    int error_code;

    auto_init_error = ft_environment_maybe_auto_initialize();
    if (auto_init_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(auto_init_error);
        return (-1);
    }
    unlock_result = ft_environment_unlock_mutex();
    if (unlock_result != FT_ERR_SUCCESSS)
        return (-1);
    if (g_environment_force_unlock_failure != 0)
    {
        forced_errno = g_environment_forced_unlock_errno;
        g_environment_force_unlock_failure = 0;
        g_environment_forced_unlock_errno = FT_ERR_SUCCESSS;
        if (forced_errno != FT_ERR_SUCCESSS)
            error_code = forced_errno;
        else
            error_code = FT_ERR_MUTEX_NOT_OWNER;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    return (0);
}

int ft_environment_enable_thread_safety(void)
{
    int result;

    result = ft_environment_enable_thread_safety_internal();
    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    g_environment_auto_initialize = true;
    return (0);
}

void ft_environment_disable_thread_safety(void)
{
    ft_environment_disable_thread_safety_internal();
    g_environment_auto_initialize = false;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
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
    g_environment_forced_lock_errno = FT_ERR_SUCCESSS;
    g_environment_forced_unlock_errno = FT_ERR_SUCCESSS;
    return ;
}

#endif
