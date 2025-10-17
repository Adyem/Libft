#include "logger_internal.hpp"

t_log_level g_level = LOG_LEVEL_INFO;
ft_vector<s_log_sink> g_sinks;
pthread_mutex_t g_sinks_mutex;
static pthread_once_t g_sinks_mutex_once = PTHREAD_ONCE_INIT;
static int g_sinks_mutex_init_error = 0;
bool g_use_color = true;

static void logger_initialize_sinks_mutex()
{
    int init_result;

    init_result = pthread_mutex_init(&g_sinks_mutex, nullptr);
    if (init_result != 0)
    {
        g_sinks_mutex_init_error = init_result;
        return ;
    }
    g_sinks_mutex_init_error = 0;
    return ;
}

int logger_lock_sinks()
{
    int once_result;
    int lock_result;

    once_result = pthread_once(&g_sinks_mutex_once, logger_initialize_sinks_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_sinks_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_sinks_mutex_init_error);
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_sinks_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int logger_unlock_sinks()
{
    int once_result;
    int unlock_result;

    once_result = pthread_once(&g_sinks_mutex_once, logger_initialize_sinks_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_sinks_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_sinks_mutex_init_error);
        return (-1);
    }
    unlock_result = pthread_mutex_unlock(&g_sinks_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
