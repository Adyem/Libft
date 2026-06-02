#include <cstdlib>

#include "logger_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t log_async_metrics_prepare_thread_safety(s_log_async_metrics *metrics)
{
    pt_mutex *mutex_pointer;
    int32_t initialize_result;

    if (!metrics)
    {
        return (FT_ERR_INTERNAL);
    }
    if (metrics->thread_safe_enabled && metrics->mutex)
    {
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new(std::nothrow) pt_mutex();
    if (!mutex_pointer)
        return (FT_ERR_INTERNAL);
    initialize_result = mutex_pointer->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (FT_ERR_INTERNAL);
    }
    metrics->mutex = mutex_pointer;
    metrics->thread_safe_enabled = FT_TRUE;
    return (FT_ERR_SUCCESS);
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
    metrics->thread_safe_enabled = FT_FALSE;
    return ;
}

int32_t log_async_metrics_lock(s_log_async_metrics *metrics, ft_bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    if (!metrics)
    {
        return (FT_ERR_INTERNAL);
    }
    if (!metrics->thread_safe_enabled || !metrics->mutex)
    {
        return (FT_ERR_SUCCESS);
    }
    if (pt_mutex_lock_if_not_null(metrics->mutex) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void log_async_metrics_unlock(s_log_async_metrics *metrics, ft_bool lock_acquired)
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
