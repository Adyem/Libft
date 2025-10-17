#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/queue.hpp"
#include "../PThread/pthread.hpp"
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"
#include <unistd.h>
#include <cerrno>

bool g_async_running = false;
static size_t g_async_queue_limit = 1024;
static size_t g_async_pending_messages = 0;
static size_t g_async_peak_pending = 0;
static size_t g_async_dropped_messages = 0;
static ft_queue<ft_string> g_log_queue;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
static pthread_mutex_t g_condition_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_queue_condition = PTHREAD_COND_INITIALIZER;
#pragma GCC diagnostic pop
static pthread_t g_log_thread;

static void ft_log_process_message(const ft_string &message)
{
    size_t sink_count;
    ft_vector<s_log_sink> sinks_snapshot;
    int    final_error;

    if (logger_lock_sinks() != 0)
        return ;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != ER_SUCCESS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return ;
        ft_errno = final_error;
        return ;
    }
    if (sink_count == 0)
    {
        if (logger_unlock_sinks() != 0)
            return ;
        ssize_t write_result;
        write_result = write(1, message.c_str(), message.size());
        (void)write_result;
        return ;
    }
    size_t index;
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;
        entry = g_sinks[index];
        if (g_sinks.get_error() != ER_SUCCESS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
            {
                return ;
            }
            ft_errno = final_error;
            return ;
        }
        sinks_snapshot.push_back(entry);
        if (sinks_snapshot.get_error() != ER_SUCCESS)
        {
            final_error = sinks_snapshot.get_error();
            if (logger_unlock_sinks() != 0)
                return ;
            ft_errno = final_error;
            return ;
        }
        index++;
    }
    if (logger_unlock_sinks() != 0)
        return ;
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[index];
        if (sinks_snapshot.get_error() != ER_SUCCESS)
        {
            ft_errno = sinks_snapshot.get_error();
            return ;
        }
        bool rotate_for_size_pre;
        bool rotate_for_age_pre;
        s_file_sink *file_sink;

        rotate_for_size_pre = false;
        rotate_for_age_pre = false;
        file_sink = ft_nullptr;
        if (entry.function == ft_file_sink)
        {
            file_sink = static_cast<s_file_sink *>(entry.user_data);
            if (logger_prepare_rotation(file_sink, &rotate_for_size_pre, &rotate_for_age_pre) != 0)
                return ;
        }
        entry.function(message.c_str(), entry.user_data);
        if (entry.function == ft_file_sink)
        {
            bool rotate_for_size_post;
            bool rotate_for_age_post;

            rotate_for_size_post = false;
            rotate_for_age_post = false;
            if (logger_prepare_rotation(file_sink, &rotate_for_size_post, &rotate_for_age_post) != 0)
                return ;
            if (rotate_for_size_pre || rotate_for_age_pre || rotate_for_size_post || rotate_for_age_post)
            {
                logger_execute_rotation(file_sink);
                if (ft_errno != ER_SUCCESS)
                    return ;
            }
        }
        index++;
    }
    return ;
}

static void *ft_log_worker(void *argument)
{
    bool queue_is_empty;
    ft_string message;
    int queue_error;

    (void)argument;
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (ft_nullptr);
    }
    while (1)
    {
        queue_is_empty = g_log_queue.empty();
        if (g_log_queue.get_error() != ER_SUCCESS)
            break;
        if (!g_async_running && queue_is_empty)
            break;
        if (queue_is_empty)
        {
            if (pt_cond_wait(&g_queue_condition, &g_condition_mutex) != 0)
                break;
        }
        else
        {
            message = g_log_queue.dequeue();
            queue_error = g_log_queue.get_error();
            if (queue_error == ER_SUCCESS)
            {
                if (g_async_pending_messages > 0)
                    g_async_pending_messages -= 1;
            }
            pthread_mutex_unlock(&g_condition_mutex);
            if (queue_error == ER_SUCCESS)
                ft_log_process_message(message);
            if (pthread_mutex_lock(&g_condition_mutex) != 0)
            {
                ft_errno = ft_map_system_error(errno);
                return (ft_nullptr);
            }
        }
    }
    pthread_mutex_unlock(&g_condition_mutex);
    return (ft_nullptr);
}

void ft_log_enable_async(bool enable)
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    if (enable)
    {
        if (g_async_running)
        {
            pthread_mutex_unlock(&g_condition_mutex);
            ft_errno = ER_SUCCESS;
            return ;
        }
        g_async_running = true;
        g_async_pending_messages = 0;
        g_async_peak_pending = 0;
        g_async_dropped_messages = 0;
        pthread_mutex_unlock(&g_condition_mutex);
        if (pt_thread_create(&g_log_thread, ft_nullptr, ft_log_worker, ft_nullptr) != 0)
        {
            if (pthread_mutex_lock(&g_condition_mutex) != 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
            g_async_running = false;
            pthread_mutex_unlock(&g_condition_mutex);
        }
        else
            ft_errno = ER_SUCCESS;
    }
    else
    {
        if (!g_async_running)
        {
            pthread_mutex_unlock(&g_condition_mutex);
            ft_errno = ER_SUCCESS;
            return ;
        }
        g_async_running = false;
        pthread_mutex_unlock(&g_condition_mutex);
        if (pt_cond_broadcast(&g_queue_condition) != 0)
            return ;
        if (pt_thread_join(g_log_thread, ft_nullptr) != 0)
            return ;
        if (pthread_mutex_lock(&g_condition_mutex) == 0)
        {
            g_async_pending_messages = 0;
            pthread_mutex_unlock(&g_condition_mutex);
        }
        ft_errno = ER_SUCCESS;
    }
    return ;
}

void ft_log_enqueue(t_log_level level, const char *fmt, va_list args)
{
    char message_buffer[1024];
    char time_buffer[32];
    char final_buffer[1200];
    t_time current_time;
    t_time_info time_info;
    int length;
    int queue_error;
    int signal_result;
    int unlock_result;

    if (!fmt)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (level < g_level)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    pf_vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);
    current_time = time_now();
    time_local(current_time, &time_info);
    time_strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);
    length = pf_snprintf(final_buffer, sizeof(final_buffer), "[%s] [%s] %s\n", time_buffer, ft_level_to_str(level), message_buffer);
    if (length <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    ft_string message(final_buffer);
    if (message.get_error() != ER_SUCCESS)
    {
        ft_errno = message.get_error();
        return ;
    }
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    if (g_async_queue_limit > 0 && g_async_pending_messages >= g_async_queue_limit)
    {
        g_async_dropped_messages += 1;
        if (pthread_mutex_unlock(&g_condition_mutex) != 0)
        {
            ft_errno = ft_map_system_error(errno);
            return ;
        }
        ft_errno = FT_ERR_FULL;
        return ;
    }
    g_log_queue.enqueue(message);
    queue_error = g_log_queue.get_error();
    if (queue_error == ER_SUCCESS)
    {
        g_async_pending_messages += 1;
        if (g_async_pending_messages > g_async_peak_pending)
            g_async_peak_pending = g_async_pending_messages;
    }
    signal_result = 0;
    if (queue_error == ER_SUCCESS)
        signal_result = pt_cond_signal(&g_queue_condition);
    unlock_result = pthread_mutex_unlock(&g_condition_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    if (queue_error != ER_SUCCESS)
    {
        ft_errno = queue_error;
        return ;
    }
    if (signal_result != 0)
        return ;
    ft_errno = ER_SUCCESS;
    return ;
}

void ft_log_set_async_queue_limit(size_t limit)
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    g_async_queue_limit = limit;
    if (g_async_queue_limit > 0)
    {
        bool needs_trim;

        needs_trim = g_async_pending_messages > g_async_queue_limit;
        while (needs_trim)
        {
            ft_string dropped_message;
            int drop_error;

            dropped_message = g_log_queue.dequeue();
            drop_error = g_log_queue.get_error();
            if (drop_error != ER_SUCCESS)
            {
                if (pthread_mutex_unlock(&g_condition_mutex) != 0)
                {
                    ft_errno = ft_map_system_error(errno);
                    return ;
                }
                ft_errno = drop_error;
                return ;
            }
            if (g_async_pending_messages > 0)
                g_async_pending_messages -= 1;
            g_async_dropped_messages += 1;
            needs_trim = g_async_pending_messages > g_async_queue_limit;
        }
    }
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

size_t ft_log_get_async_queue_limit()
{
    size_t limit;

    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (0);
    }
    limit = g_async_queue_limit;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (limit);
}

int ft_log_get_async_metrics(s_log_async_metrics *metrics)
{
    if (!metrics)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    metrics->pending_messages = g_async_pending_messages;
    metrics->peak_pending_messages = g_async_peak_pending;
    metrics->dropped_messages = g_async_dropped_messages;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

void ft_log_reset_async_metrics()
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    g_async_peak_pending = g_async_pending_messages;
    g_async_dropped_messages = 0;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}
