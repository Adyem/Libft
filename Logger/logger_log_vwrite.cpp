#include "logger_internal.hpp"
#include <cerrno>
#include <unistd.h>
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"

void ft_log_vwrite(t_log_level level, const char *fmt, va_list args)
{
    if (!fmt)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    if (level < g_level)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }

    ft_errno = ER_SUCCESS;

    char message_buffer[1024];
    pf_vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);

    char time_buffer[32];
    t_time current_time;
    t_time_info time_info;

    current_time = time_now();
    time_local(current_time, &time_info);
    time_strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);

    size_t sink_count;
    bool use_color;
    const char *color_code;
    int length;
    char final_buffer[1200];
    ft_vector<s_log_sink> sinks_snapshot;
    int final_error;

    g_sinks_mutex.lock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return ;
    }
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != ER_SUCCESS)
    {
        final_error = g_sinks.get_error();
        g_sinks_mutex.unlock(THREAD_ID);
        if (g_sinks_mutex.get_error() != ER_SUCCESS)
        {
            return ;
        }
        ft_errno = final_error;
        return ;
    }
    use_color = false;
    if (g_use_color && sink_count == 0 && isatty(1))
        use_color = true;
    color_code = "";
    if (use_color)
    {
        if (level == LOG_LEVEL_DEBUG)
            color_code = "\x1b[36m";
        else if (level == LOG_LEVEL_INFO)
            color_code = "\x1b[32m";
        else if (level == LOG_LEVEL_WARN)
            color_code = "\x1b[33m";
        else
            color_code = "\x1b[31m";
        length = pf_snprintf(final_buffer, sizeof(final_buffer), "%s[%s] [%s] %s\x1b[0m\n", color_code, time_buffer, ft_level_to_str(level), message_buffer);
    }
    else
    length = pf_snprintf(final_buffer, sizeof(final_buffer), "[%s] [%s] %s\n", time_buffer, ft_level_to_str(level), message_buffer);
    if (length <= 0)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    if (sink_count != 0)
    {
        size_t index;

        index = 0;
        while (index < sink_count)
        {
            s_log_sink entry;

            entry = g_sinks[index];
            if (g_sinks.get_error() != ER_SUCCESS)
            {
                final_error = g_sinks.get_error();
                g_sinks_mutex.unlock(THREAD_ID);
                if (g_sinks_mutex.get_error() != ER_SUCCESS)
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
                g_sinks_mutex.unlock(THREAD_ID);
                if (g_sinks_mutex.get_error() != ER_SUCCESS)
                {
                    return ;
                }
                ft_errno = final_error;
                return ;
            }
            index++;
        }
    }
    g_sinks_mutex.unlock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return ;
    }
    if (sink_count == 0)
    {
        ssize_t write_result;

        write_result = write(1, final_buffer, static_cast<size_t>(length));
        if (write_result < 0)
        {
            ft_errno = errno + ERRNO_OFFSET;
            return ;
        }
        ft_errno = ER_SUCCESS;
        return ;
    }
    size_t index;

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
        entry.function(final_buffer, entry.user_data);
        if (ft_errno != ER_SUCCESS)
            return ;
        if (entry.function == ft_file_sink)
        {
            ft_log_rotate(static_cast<s_file_sink *>(entry.user_data));
            if (ft_errno != ER_SUCCESS)
                return ;
        }
        index++;
    }
    ft_errno = ER_SUCCESS;
    return ;
}
