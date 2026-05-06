#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_lock_tracking.hpp"
#include "../Time/time.hpp"

static pthread_mutex_t g_lock_contention_mutex;
static pthread_once_t g_lock_contention_once = PTHREAD_ONCE_INIT;
static int32_t g_lock_contention_mutex_init_error = 0;
static ft_bool g_lock_contention_sampling_enabled = FT_TRUE;
static uint32_t g_lock_contention_sample_interval_ms = 250;
static int64_t g_lock_contention_last_sample_ms = 0;
static int64_t g_lock_contention_priority_threshold_ms = 500;
static ft_size_t g_lock_contention_total_samples = 0;
static ft_size_t g_lock_contention_priority_inversions = 0;
static ft_size_t g_lock_contention_skipped_samples = 0;
static double g_lock_contention_wait_total_ms = 0.0;
static ft_size_t g_lock_contention_wait_observations = 0;
static int64_t g_lock_contention_longest_wait_ms = 0;

static void logger_lock_contention_initialize_mutex()
{
    int32_t initialization_status;

    initialization_status = pthread_mutex_init(&g_lock_contention_mutex, ft_nullptr);
    if (initialization_status != 0)
    {
        g_lock_contention_mutex_init_error = initialization_status;
        return ;
    }
    g_lock_contention_mutex_init_error = 0;
    return ;
}

static int32_t logger_lock_contention_lock()
{
    int32_t once_status;
    int32_t lock_status;

    once_status = pthread_once(&g_lock_contention_once, logger_lock_contention_initialize_mutex);
    if (once_status != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    if (g_lock_contention_mutex_init_error != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    lock_status = pthread_mutex_lock(&g_lock_contention_mutex);
    if (lock_status != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t logger_lock_contention_unlock()
{
    int32_t once_status;
    int32_t unlock_status;

    once_status = pthread_once(&g_lock_contention_once, logger_lock_contention_initialize_mutex);
    if (once_status != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    if (g_lock_contention_mutex_init_error != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    unlock_status = pthread_mutex_unlock(&g_lock_contention_mutex);
    if (unlock_status != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

void ft_log_enable_lock_contention_sampling(ft_bool enable)
{
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_sampling_enabled = enable;
    if (!g_lock_contention_sampling_enabled)
        g_lock_contention_last_sample_ms = 0;
    logger_lock_contention_unlock();
    return ;
}

void ft_log_set_lock_contention_sampling_interval(uint32_t interval_ms)
{
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_sample_interval_ms = interval_ms;
    logger_lock_contention_unlock();
    return ;
}

void ft_log_set_lock_contention_priority_threshold(int64_t threshold_ms)
{
    if (threshold_ms < 0)
        threshold_ms = 0;
    if (logger_lock_contention_lock() != 0)
        return ;
    g_lock_contention_priority_threshold_ms = threshold_ms;
    logger_lock_contention_unlock();
    return ;
}

static void logger_lock_contention_record_sample(int64_t wait_duration_ms, ft_bool priority_inversion, ft_bool skipped)
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

int32_t ft_log_sample_lock_contention(s_log_lock_contention_sample *samples, ft_size_t capacity, ft_size_t *entry_count)
{
    pt_lock_wait_snapshot_vector waiters;
    ft_size_t entry_index;
    ft_size_t output_index;
    int64_t now_ms;

    if (entry_count)
        *entry_count = 0;
    if ((samples == ft_nullptr && capacity > 0) || capacity == 0)
    {
        return (FT_ERR_INTERNAL);
    }
    if (logger_lock_contention_lock() != 0)
        return (FT_ERR_INTERNAL);
    if (!g_lock_contention_sampling_enabled)
    {
        logger_lock_contention_unlock();
        return (FT_ERR_SUCCESS);
    }
    now_ms = time_now_ms();
    if (g_lock_contention_sample_interval_ms > 0 && g_lock_contention_last_sample_ms != 0)
    {
        int64_t elapsed_ms;

        elapsed_ms = now_ms - g_lock_contention_last_sample_ms;
        if (elapsed_ms >= 0 && elapsed_ms < static_cast<int64_t>(g_lock_contention_sample_interval_ms))
        {
            logger_lock_contention_unlock();
            return (FT_ERR_SUCCESS);
        }
    }
    g_lock_contention_last_sample_ms = now_ms;
    if (!pt_lock_tracking::snapshot_waiters(waiters))
    {
        logger_lock_contention_unlock();
        return (FT_ERR_INTERNAL);
    }
    entry_index = 0;
    output_index = 0;
    while (entry_index < waiters.size)
    {
        s_pt_lock_wait_snapshot snapshot_entry;
        int64_t wait_duration_ms;
        ft_bool priority_inversion;

        snapshot_entry = waiters.data[entry_index];
        wait_duration_ms = 0;
        if (snapshot_entry.wait_started_ms > 0 && now_ms >= snapshot_entry.wait_started_ms)
            wait_duration_ms = now_ms - snapshot_entry.wait_started_ms;
        priority_inversion = FT_FALSE;
        if (wait_duration_ms >= g_lock_contention_priority_threshold_ms
            && snapshot_entry.owner_thread != 0 && snapshot_entry.waiting_thread != 0)
        {
            priority_inversion = FT_TRUE;
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
            logger_lock_contention_record_sample(wait_duration_ms, priority_inversion, FT_FALSE);
            output_index += 1;
        }
        else
            logger_lock_contention_record_sample(wait_duration_ms, priority_inversion, FT_TRUE);
        entry_index += 1;
    }
    if (entry_count)
        *entry_count = output_index;
    logger_lock_contention_unlock();
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_lock_contention_get_statistics(s_log_lock_contention_statistics *statistics)
{
    double average_ms;

    if (!statistics)
    {
        return (FT_ERR_INTERNAL);
    }
    if (logger_lock_contention_lock() != 0)
        return (FT_ERR_INTERNAL);
    statistics->total_samples = g_lock_contention_total_samples;
    statistics->priority_inversions = g_lock_contention_priority_inversions;
    statistics->skipped_samples = g_lock_contention_skipped_samples;
    statistics->longest_wait_ms = g_lock_contention_longest_wait_ms;
    average_ms = 0.0;
    if (g_lock_contention_wait_observations > 0)
        average_ms = g_lock_contention_wait_total_ms / static_cast<double>(g_lock_contention_wait_observations);
    statistics->average_wait_ms = average_ms;
    logger_lock_contention_unlock();
    return (FT_ERR_SUCCESS);
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
    logger_lock_contention_unlock();
    return ;
}
