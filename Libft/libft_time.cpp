#include "libft.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include <sys/time.h>
#include <time.h>

long ft_time_ms(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, ft_nullptr) != 0)
        return (-1);
    return ((tv.tv_sec * 1000L + tv.tv_usec / 1000L));
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm *tm_info;

    if (!buffer || buffer_size == 0)
        return (ft_nullptr);
    now = time(ft_nullptr);
    tm_info = localtime(&now);
    if (!tm_info)
        return (ft_nullptr);
    if (strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info) == 0)
        return (ft_nullptr);
    return (buffer);
}
