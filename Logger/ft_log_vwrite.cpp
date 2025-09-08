#include "logger_internal.hpp"
#include <unistd.h>
#include <cstdio>
#include <ctime>

void ft_log_vwrite(t_log_level level, const char *fmt, va_list args)
{
    if (level < g_level || !fmt)
        return ;

    char msg_buf[1024];
    std::vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);

    char time_buf[32];
    std::time_t t = std::time(NULL);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);

    char final_buf[1200];
    int len = std::snprintf(final_buf, sizeof(final_buf), "[%s] [%s] %s\n", time_buf, ft_level_to_str(level), msg_buf);
    if (len > 0)
    {
        ssize_t write_result = write(g_fd, final_buf, static_cast<size_t>(len));
        (void)write_result;
    }
    ft_log_rotate();
}
