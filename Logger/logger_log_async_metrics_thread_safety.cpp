#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int log_async_metrics_prepare_thread_safety(s_log_async_metrics *metrics)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!metrics)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (metrics->thread_safe_enabled && metrics->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    metrics->mutex = mutex_pointer;
    metrics->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void log_async_metrics_teardown_thread_safety(s_log_async_metrics *metrics)
{
    if (!metrics)
        return ;
    if (metrics->mutex)
    {
        metrics->mutex->~pt_mutex();
        std::free(metrics->mutex);
        metrics->mutex = ft_nullptr;
    }
    metrics->thread_safe_enabled = false;
    return ;
}

int log_async_metrics_lock(s_log_async_metrics *metrics, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!metrics)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!metrics->thread_safe_enabled || !metrics->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    metrics->mutex->lock(THREAD_ID);
    if (metrics->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = metrics->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void log_async_metrics_unlock(s_log_async_metrics *metrics, bool lock_acquired)
{
    if (!metrics)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (!lock_acquired || !metrics->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    metrics->mutex->unlock(THREAD_ID);
    if (metrics->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = metrics->mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
