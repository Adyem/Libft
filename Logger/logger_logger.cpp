#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/queue.hpp"
#include "../PThread/pthread.hpp"
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"
#include <unistd.h>
#include <cerrno>

ft_logger *g_logger = ft_nullptr;
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

void ft_log_start_async()
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    if (g_async_running)
    {
        pthread_mutex_unlock(&g_condition_mutex);
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
    return ;
}

void ft_log_stop_async()
{
    if (pthread_mutex_lock(&g_condition_mutex) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    if (!g_async_running)
    {
        pthread_mutex_unlock(&g_condition_mutex);
        return ;
    }
    g_async_running = false;
    pthread_mutex_unlock(&g_condition_mutex);
    if (pt_cond_broadcast(&g_queue_condition) != 0)
        return ;
    if (pt_thread_join(g_log_thread, ft_nullptr) != 0)
        return ;
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

ft_logger::ft_logger(const char *path, size_t max_size, t_log_level level) noexcept
    : _alloc_logging(false), _api_logging(false)
{
    ft_log_set_level(level);
    if (path)
        ft_log_set_file(path, max_size);
}

ft_logger::~ft_logger() noexcept
{
    ft_log_close();
    if (g_logger == this)
        g_logger = ft_nullptr;
}

void ft_logger::set_global() noexcept
{
    g_logger = this;
}

void ft_logger::set_level(t_log_level level) noexcept
{
    ft_log_set_level(level);
}

int ft_logger::set_file(const char *path, size_t max_size) noexcept
{
    return (ft_log_set_file(path, max_size));
}

int ft_logger::add_sink(t_log_sink sink, void *user_data) noexcept
{
    return (ft_log_add_sink(sink, user_data));
}

void ft_logger::remove_sink(t_log_sink sink, void *user_data) noexcept
{
    ft_log_remove_sink(sink, user_data);
    return ;
}

void ft_logger::set_alloc_logging(bool enable) noexcept
{
    this->_alloc_logging = enable;
}

bool ft_logger::get_alloc_logging() const noexcept
{
    return (this->_alloc_logging);
}

void ft_logger::set_api_logging(bool enable) noexcept
{
    this->_api_logging = enable;
}

bool ft_logger::get_api_logging() const noexcept
{
    return (this->_api_logging);
}

void ft_logger::set_color(bool enable) noexcept
{
    ft_log_set_color(enable);
    return ;
}

bool ft_logger::get_color() const noexcept
{
    return (ft_log_get_color());
}

void ft_logger::close() noexcept
{
    ft_log_close();
}

void ft_logger::debug(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_DEBUG, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void ft_logger::info(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_INFO, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void ft_logger::warn(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_WARN, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void ft_logger::error(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    if (g_async_running)
        ft_log_enqueue(LOG_LEVEL_ERROR, fmt, args);
    else
        ft_log_vwrite(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

