#include "logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <ctime>
#include <cstdio>
#include <unistd.h>

ft_logger *g_logger = ft_nullptr;
bool g_async_running = false;
static std::queue<std::string> g_log_queue;
static std::mutex g_queue_mutex;
static std::condition_variable g_queue_cv;
static std::thread g_log_thread;

static void ft_log_process_message(const std::string &message)
{
    if (g_sinks.empty())
    {
        ssize_t write_result;

        write_result = write(1, message.c_str(), message.size());
        (void)write_result;
    }
    else
    {
        size_t index;

        index = 0;
        while (index < g_sinks.size())
        {
            g_sinks[index].function(message.c_str(), g_sinks[index].user_data);
            if (g_sinks[index].function == ft_file_sink)
                ft_log_rotate(static_cast<s_file_sink *>(g_sinks[index].user_data));
            index++;
        }
    }
    return ;
}

static void ft_log_worker()
{
    std::unique_lock<std::mutex> queue_lock(g_queue_mutex);
    while (g_async_running || !g_log_queue.empty())
    {
        if (g_log_queue.empty())
            g_queue_cv.wait(queue_lock);
        else
        {
            std::string message;

            message = g_log_queue.front();
            g_log_queue.pop();
            queue_lock.unlock();
            ft_log_process_message(message);
            queue_lock.lock();
        }
    }
    return ;
}

void ft_log_start_async()
{
    if (g_async_running)
        return ;
    g_async_running = true;
    g_log_thread = std::thread(ft_log_worker);
    return ;
}

void ft_log_stop_async()
{
    if (!g_async_running)
        return ;
    {
        std::lock_guard<std::mutex> lock_guard(g_queue_mutex);
        g_async_running = false;
    }
    g_queue_cv.notify_one();
    if (g_log_thread.joinable())
        g_log_thread.join();
    return ;
}

void ft_log_enqueue(t_log_level level, const char *fmt, va_list args)
{
    if (level < g_level || !fmt)
        return ;
    char message_buffer[1024];
    std::vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);

    char time_buffer[32];
    std::time_t current_time;
    std::tm time_info;

    current_time = std::time(NULL);
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&time_info, &current_time);
#else
    localtime_r(&current_time, &time_info);
#endif
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);

    char final_buffer[1200];
    std::snprintf(final_buffer, sizeof(final_buffer), "[%s] [%s] %s\n", time_buffer, ft_level_to_str(level), message_buffer);

    {
        std::lock_guard<std::mutex> lock_guard(g_queue_mutex);
        g_log_queue.push(final_buffer);
    }
    g_queue_cv.notify_one();
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

