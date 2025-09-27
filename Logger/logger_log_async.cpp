#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/queue.hpp"
#include "../PThread/pthread.hpp"
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"
#include <unistd.h>
#include <cerrno>

bool g_async_running = false;
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

    sink_count = g_sinks.size();
    if (sink_count == 0)
    {
        ssize_t write_result;

        write_result = write(1, message.c_str(), message.size());
        (void)write_result;
    }
    else
    {
        size_t index;

        index = 0;
        while (index < sink_count)
        {
            g_sinks[index].function(message.c_str(), g_sinks[index].user_data);
            if (g_sinks[index].function == ft_file_sink)
                ft_log_rotate(static_cast<s_file_sink *>(g_sinks[index].user_data));
            index++;
        }
    }
    return ;
}

static void *ft_log_worker(void *argument)
{
    bool queue_is_empty;
    ft_string message;

    (void)argument;
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
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
            pthread_mutex_unlock(&g_condition_mutex);
            if (g_log_queue.get_error() == ER_SUCCESS)
                ft_log_process_message(message);
            if (pthread_mutex_lock(&g_condition_mutex) != 0)
            {
                ft_errno = errno + ERRNO_OFFSET;
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
        ft_errno = errno + ERRNO_OFFSET;
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
        pthread_mutex_unlock(&g_condition_mutex);
        if (pt_thread_create(&g_log_thread, ft_nullptr, ft_log_worker, ft_nullptr) != 0)
        {
            if (pthread_mutex_lock(&g_condition_mutex) != 0)
            {
                ft_errno = errno + ERRNO_OFFSET;
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
        ft_errno = ER_SUCCESS;
    }
    return ;
}

void ft_log_enqueue(t_log_level level, const char *fmt, va_list args)
{
    if (level < g_level || !fmt)
        return ;
    char message_buffer[1024];
    pf_vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);

    char time_buffer[32];
    t_time current_time;
    t_time_info time_info;

    current_time = time_now();
    time_local(current_time, &time_info);
    time_strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);

    char final_buffer[1200];
    int length;

    length = pf_snprintf(final_buffer, sizeof(final_buffer), "[%s] [%s] %s\n", time_buffer, ft_level_to_str(level), message_buffer);
    if (length <= 0)
        return ;
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    g_log_queue.enqueue(ft_string(final_buffer));
    if (g_log_queue.get_error() == ER_SUCCESS)
        pt_cond_signal(&g_queue_condition);
    pthread_mutex_unlock(&g_condition_mutex);
    return ;
}
