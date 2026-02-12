#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_lock_tracking.hpp"
#include "../Time/time.hpp"

static pthread_mutex_t g_lock_contention_mutex;
static pthread_once_t g_lock_contention_once = PTHREAD_ONCE_INIT;
static int g_lock_contention_mutex_init_error = 0;
static bool g_lock_contention_sampling_enabled = true;
static unsigned int g_lock_contention_sample_interval_ms = 250;
static long g_lock_contention_last_sample_ms = 0;
static long g_lock_contention_priority_threshold_ms = 500;
static size_t g_lock_contention_total_samples = 0;
static size_t g_lock_contention_priority_inversions = 0;
static size_t g_lock_contention_skipped_samples = 0;
static double g_lock_contention_wait_total_ms = 0.0;
static size_t g_lock_contention_wait_observations = 0;
static long g_lock_contention_longest_wait_ms = 0;

static void logger_lock_contention_initialize_mutex()
{
    int init_result;

    init_result = pthread_mutex_init(&g_lock_contention_mutex, ft_nullptr);
    if (init_result != 0)
    {
        g_lock_contention_mutex_init_error = init_result;
        return ;
    }
    g_lock_contention_mutex_init_error = 0;
    return ;
}

static int logger_lock_contention_lock()
{
    int once_result;
    int lock_result;

    once_result = pthread_once(&g_lock_contention_once, logger_lock_contention_initialize_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_lock_contention_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_lock_contention_mutex_init_error);
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_lock_contention_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESS;
    return (0);
}

static int logger_lock_contention_unlock()
{
    int once_result;
    int unlock_result;

    once_result = pthread_once(&g_lock_contention_once, logger_lock_contention_initialize_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_lock_contention_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_lock_contention_mutex_init_error);
        return (-1);
    }
    unlock_result = pthread_mutex_unlock(&g_lock_contention_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESS;
    return (0);
}

void ft_log_enable_lock_contention_sampling(bool enable)
{
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_sampling_enabled = enable;
    if (!g_lock_contention_sampling_enabled)
        g_lock_contention_last_sample_ms = 0;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return ;
}

void ft_log_set_lock_contention_sampling_interval(unsigned int interval_ms)
{
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_sample_interval_ms = interval_ms;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return ;
}

void ft_log_set_lock_contention_priority_threshold(long threshold_ms)
{
    if (threshold_ms < 0)
        threshold_ms = 0;
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_priority_threshold_ms = threshold_ms;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return ;
}

static void logger_lock_contention_record_sample(long wait_duration_ms, bool priority_inversion, bool skipped)
{
    g_lock_contention_total_samples += 1;
    if (priority_inversion)
        g_lock_contention_priority_inversions += 1;
    if (skipped)
        g_lock_contention_skipped_samples += 1;
    if (wait_duration_ms > g_lock_contention_longest_wait_ms)
        g_lock_contention_longest_wait_ms = wait_duration_ms;
    g_lock_contention_wait_total_ms += static_cast<double>(wait_duration_ms);
    g_lock_contention_wait_observations += 1;
    return ;
}

int ft_log_sample_lock_contention(s_log_lock_contention_sample *samples, size_t capacity, size_t *count)
{
    pt_lock_wait_snapshot_vector waiters;
    size_t index;
    size_t output_index;
    long now_ms;

    if (count)
        *count = 0;
    if ((samples == ft_nullptr && capacity > 0) || capacity == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (logger_lock_contention_lock() != 0)
        return (-1);
    if (!g_lock_contention_sampling_enabled)
    {
        ft_errno = FT_ERR_SUCCESS;
        logger_lock_contention_unlock();
        return (0);
    }
    now_ms = time_now_ms();
    if (g_lock_contention_sample_interval_ms > 0 && g_lock_contention_last_sample_ms != 0)
    {
        long elapsed_ms;

        elapsed_ms = now_ms - g_lock_contention_last_sample_ms;
        if (elapsed_ms >= 0 && elapsed_ms < static_cast<long>(g_lock_contention_sample_interval_ms))
        {
            ft_errno = FT_ERR_SUCCESS;
            logger_lock_contention_unlock();
            return (0);
        }
    }
    g_lock_contention_last_sample_ms = now_ms;
    if (!pt_lock_tracking::snapshot_waiters(waiters))
    {
        int tracker_error;

        tracker_error = ft_errno;
        logger_lock_contention_unlock();
        ft_errno = tracker_error;
        return (-1);
    }
    index = 0;
    output_index = 0;
    while (index < waiters.size())
    {
        s_pt_lock_wait_snapshot snapshot_entry;
        long wait_duration_ms;
        bool priority_inversion;

        snapshot_entry = waiters[index];
        wait_duration_ms = 0;
        if (snapshot_entry.wait_started_ms > 0 && now_ms >= snapshot_entry.wait_started_ms)
            wait_duration_ms = now_ms - snapshot_entry.wait_started_ms;
        priority_inversion = false;
        if (wait_duration_ms >= g_lock_contention_priority_threshold_ms
            && snapshot_entry.owner_thread != 0 && snapshot_entry.waiting_thread != 0)
        {
            priority_inversion = true;
        }
        if (output_index < capacity)
        {
            s_log_lock_contention_sample sample;

            sample.mutex_pointer = snapshot_entry.mutex_pointer;
            sample.owner_thread = snapshot_entry.owner_thread;
            sample.waiting_thread = snapshot_entry.waiting_thread;
            sample.wait_duration_ms = wait_duration_ms;
            sample.priority_inversion = priority_inversion;
            samples[output_index] = sample;
            logger_lock_contention_record_sample(wait_duration_ms, priority_inversion, false);
            output_index += 1;
        }
        else
            logger_lock_contention_record_sample(wait_duration_ms, priority_inversion, true);
        index += 1;
    }
    if (count)
        *count = output_index;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return (0);
}

int ft_log_lock_contention_get_statistics(s_log_lock_contention_statistics *statistics)
{
    double average_ms;

    if (!statistics)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (logger_lock_contention_lock() != 0)
        return (-1);
    statistics->total_samples = g_lock_contention_total_samples;
    statistics->priority_inversions = g_lock_contention_priority_inversions;
    statistics->skipped_samples = g_lock_contention_skipped_samples;
    statistics->longest_wait_ms = g_lock_contention_longest_wait_ms;
    average_ms = 0.0;
    if (g_lock_contention_wait_observations > 0)
        average_ms = g_lock_contention_wait_total_ms / static_cast<double>(g_lock_contention_wait_observations);
    statistics->average_wait_ms = average_ms;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return (0);
}

void ft_log_lock_contention_reset_statistics()
{
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_total_samples = 0;
    g_lock_contention_priority_inversions = 0;
    g_lock_contention_skipped_samples = 0;
    g_lock_contention_wait_total_ms = 0.0;
    g_lock_contention_wait_observations = 0;
    g_lock_contention_longest_wait_ms = 0;
    ft_errno = FT_ERR_SUCCESS;
    logger_lock_contention_unlock();
    return ;
}
