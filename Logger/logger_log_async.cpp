#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/queue.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/condition.hpp"
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
    int    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        logger_unlock_sinks();
        return ;
    }
    if (sink_count == 0)
    {
        logger_unlock_sinks();
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
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            logger_unlock_sinks();
            return ;
        }
        sinks_snapshot.push_back(entry);
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            logger_unlock_sinks();
            return ;
        }
        index++;
    }
    logger_unlock_sinks();
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[index];
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
            return ;
        bool sink_lock_acquired;
        int  sink_error;
        int  sink_lock_error;

        sink_lock_acquired = false;
        sink_lock_error = log_sink_lock(&entry, &sink_lock_acquired);
        sink_error = sink_lock_error;
        if (sink_error == FT_ERR_SUCCESSS)
        {
            bool        rotate_for_size_pre;
            bool        rotate_for_age_pre;
            s_file_sink *file_sink;

            rotate_for_size_pre = false;
            rotate_for_age_pre = false;
            file_sink = ft_nullptr;
            if (entry.function == ft_file_sink)
            {
                file_sink = static_cast<s_file_sink *>(entry.user_data);
                if (logger_prepare_rotation(file_sink, &rotate_for_size_pre, &rotate_for_age_pre) != 0)
                    sink_error = FT_ERR_INVALID_OPERATION;
            }
            if (sink_error == FT_ERR_SUCCESSS)
                entry.function(message.c_str(), entry.user_data);
            if (sink_error == FT_ERR_SUCCESSS && entry.function == ft_file_sink)
            {
                bool rotate_for_size_post;
                bool rotate_for_age_post;

                rotate_for_size_post = false;
                rotate_for_age_post = false;
                if (logger_prepare_rotation(file_sink, &rotate_for_size_post, &rotate_for_age_post) != 0)
                    sink_error = FT_ERR_INVALID_OPERATION;
                else if (rotate_for_size_pre || rotate_for_age_pre || rotate_for_size_post || rotate_for_age_post)
                {
                    logger_execute_rotation(file_sink);
                }
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&entry, sink_lock_acquired);
        if (sink_error != FT_ERR_SUCCESSS)
            return ;
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
        return (ft_nullptr);
    while (1)
    {
        queue_is_empty = g_log_queue.empty();
        if (g_log_queue.get_error() != FT_ERR_SUCCESSS)
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
            if (queue_error == FT_ERR_SUCCESSS)
            {
                if (g_async_pending_messages > 0)
                    g_async_pending_messages -= 1;
            }
            pthread_mutex_unlock(&g_condition_mutex);
            if (queue_error == FT_ERR_SUCCESSS)
                ft_log_process_message(message);
            if (pthread_mutex_lock(&g_condition_mutex) != 0)
                return (ft_nullptr);
        }
    }
    pthread_mutex_unlock(&g_condition_mutex);
    return (ft_nullptr);
}

void ft_log_enable_async(bool enable)
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    if (enable)
    {
        if (g_async_running)
        {
            pthread_mutex_unlock(&g_condition_mutex);
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
                ft_global_error_stack_push(ft_map_system_error(errno));
                return ;
            }
            g_async_running = false;
            pthread_mutex_unlock(&g_condition_mutex);
        }
        else
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
    }
    else
    {
        if (!g_async_running)
        {
            pthread_mutex_unlock(&g_condition_mutex);
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    }
    return ;
}

void ft_log_enqueue(t_log_level level, const char *fmt, va_list args)
{
    ft_vector<s_redaction_rule> redaction_snapshot;
    ft_string message_text;
    ft_string context_fragment;
    ft_string final_message;
    int queue_error;
    int signal_result;
    int unlock_result;
    int format_error;
    char message_buffer[1024];
    va_list args_copy;
    int lock_error;
    int redaction_error;

    if (!fmt)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (level < g_level)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    va_copy(args_copy, args);
    int formatted_length = pf_vsnprintf(message_buffer, sizeof(message_buffer), fmt, args_copy);
    va_end(args_copy);
    format_error = ft_global_error_stack_pop_newest();
    if (formatted_length < 0)
    {
        if (format_error != FT_ERR_SUCCESSS)
            ft_global_error_stack_push(format_error);
        return ;
    }
    message_text = ft_string(message_buffer);
    if (message_text.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(message_text.get_error());
        return ;
    }
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    redaction_error = logger_copy_redaction_rules(redaction_snapshot);
    if (redaction_error != FT_ERR_SUCCESSS)
    {
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lock_error);
            return ;
        }
        ft_global_error_stack_push(redaction_error);
        return ;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    redaction_error = logger_apply_redactions(message_text, redaction_snapshot);
    if (redaction_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(redaction_error);
        return ;
    }
    if (logger_context_format_flat(context_fragment) != 0)
        return ;
    if (context_fragment.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(context_fragment.get_error());
        return ;
    }
    if (context_fragment.size() > 0)
    {
        if (logger_apply_redactions(context_fragment, redaction_snapshot) != 0)
            return ;
    }
    if (logger_build_standard_message(level, message_text, context_fragment, final_message) != 0)
        return ;
    if (final_message.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(final_message.get_error());
        return ;
    }
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    if (g_async_queue_limit > 0 && g_async_pending_messages >= g_async_queue_limit)
    {
        g_async_dropped_messages += 1;
        if (pthread_mutex_unlock(&g_condition_mutex) != 0)
        {
            ft_global_error_stack_push(ft_map_system_error(errno));
            return ;
        }
        ft_global_error_stack_push(FT_ERR_FULL);
        return ;
    }
    g_log_queue.enqueue(final_message);
    queue_error = g_log_queue.get_error();
    if (queue_error == FT_ERR_SUCCESSS)
    {
        g_async_pending_messages += 1;
        if (g_async_pending_messages > g_async_peak_pending)
            g_async_peak_pending = g_async_pending_messages;
    }
    signal_result = 0;
    if (queue_error == FT_ERR_SUCCESSS)
        signal_result = pt_cond_signal(&g_queue_condition);
    unlock_result = pthread_mutex_unlock(&g_condition_mutex);
    if (unlock_result != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    if (queue_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(queue_error);
        return ;
    }
    if (signal_result != 0)
        return ;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

void ft_log_set_async_queue_limit(size_t limit)
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
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
            if (drop_error != FT_ERR_SUCCESSS)
            {
                if (pthread_mutex_unlock(&g_condition_mutex) != 0)
                {
                    ft_global_error_stack_push(ft_map_system_error(errno));
                    return ;
                }
                ft_global_error_stack_push(drop_error);
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
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

size_t ft_log_get_async_queue_limit()
{
    size_t limit;

    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (0);
    }
    limit = g_async_queue_limit;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (limit);
}

int ft_log_get_async_metrics(s_log_async_metrics *metrics)
{
    bool metrics_lock_acquired;

    if (!metrics)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    metrics_lock_acquired = false;
    if (log_async_metrics_lock(metrics, &metrics_lock_acquired) != 0)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        log_async_metrics_unlock(metrics, metrics_lock_acquired);
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    metrics->pending_messages = g_async_pending_messages;
    metrics->peak_pending_messages = g_async_peak_pending;
    metrics->dropped_messages = g_async_dropped_messages;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        log_async_metrics_unlock(metrics, metrics_lock_acquired);
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    log_async_metrics_unlock(metrics, metrics_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void ft_log_reset_async_metrics()
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    g_async_peak_pending = g_async_pending_messages;
    g_async_dropped_messages = 0;
    if (pthread_mutex_unlock(&g_condition_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
