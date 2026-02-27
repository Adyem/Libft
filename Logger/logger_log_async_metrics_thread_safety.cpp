#include <cstdlib>

#include "logger_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"

int log_async_metrics_prepare_thread_safety(s_log_async_metrics *metrics)
{
    pt_mutex *mutex_pointer;
    int initialize_result;

    if (!metrics)
    {
        return (-1);
    }
    if (metrics->thread_safe_enabled && metrics->mutex)
    {
        return (0);
    }
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (-1);
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    metrics->mutex = mutex_pointer;
    metrics->thread_safe_enabled = true;
    return (0);
}

void log_async_metrics_teardown_thread_safety(s_log_async_metrics *metrics)
{
    if (!metrics)
        return ;
    if (metrics->mutex)
    {
        (void)metrics->mutex->destroy();
        delete metrics->mutex;
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
        return (-1);
    }
    if (!metrics->thread_safe_enabled || !metrics->mutex)
    {
        return (0);
    }
    if (pt_mutex_lock_if_not_null(metrics->mutex) != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired)
        *lock_acquired = true;
    return (0);
}

void log_async_metrics_unlock(s_log_async_metrics *metrics, bool lock_acquired)
{
    if (!metrics)
    {
        return ;
    }
    if (!lock_acquired || !metrics->mutex)
    {
        return ;
    }
    (void)pt_mutex_unlock_if_not_null(metrics->mutex);
    return ;
}
