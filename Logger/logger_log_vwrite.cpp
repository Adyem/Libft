#include "logger_internal.hpp"
#include <unistd.h>
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"

void ft_log_vwrite(t_log_level level, const char *fmt, va_list args)
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

    size_t sink_count;
    bool use_color;
    const char *color_code;
    int length;
    char final_buffer[1200];

    sink_count = g_sinks.size();
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
    if (length > 0)
    {
        if (sink_count == 0)
        {
            ssize_t write_result;

            write_result = write(1, final_buffer, static_cast<size_t>(length));
            (void)write_result;
        }
        else
        {
            size_t index;

            index = 0;
            while (index < sink_count)
            {
                g_sinks[index].function(final_buffer, g_sinks[index].user_data);
                if (g_sinks[index].function == ft_file_sink)
                    ft_log_rotate(static_cast<s_file_sink *>(g_sinks[index].user_data));
                index++;
            }
        }
    }
    return ;
}
