#include "logger_internal.hpp"

t_log_level g_level = LOG_LEVEL_INFO;
ft_vector<s_log_sink> g_sinks;
pthread_mutex_t g_sinks_mutex = PTHREAD_MUTEX_INITIALIZER;
bool g_use_color = true;

int logger_lock_sinks()
{
    int lock_result;

    lock_result = pthread_mutex_lock(&g_sinks_mutex);
    if (lock_result != 0)
    {
        ft_errno = lock_result + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int logger_unlock_sinks()
{
    int unlock_result;

    unlock_result = pthread_mutex_unlock(&g_sinks_mutex);
    if (unlock_result != 0)
    {
        ft_errno = unlock_result + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
