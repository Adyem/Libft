#include <cstdlib>

#include "logger_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int log_async_metrics_prepare_thread_safety(s_log_async_metrics *metrics)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!metrics)
    {
        return (-1);
    }
    if (metrics->thread_safe_enabled && metrics->mutex)
    {
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
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
        return (-1);
    }
    if (!metrics->thread_safe_enabled || !metrics->mutex)
    {
        return (0);
    }
    metrics->mutex->lock(THREAD_ID);
    if (metrics->mutex->get_error() != FT_ERR_SUCCESS)
    {
        return (-1);
    }
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
    metrics->mutex->unlock(THREAD_ID);
    if (metrics->mutex->get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    return ;
}
