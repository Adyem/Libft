#include "libft.hpp"
#include <sys/time.h>
#include <time.h>

long ft_time_ms(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        return (-1);
    return ((long)(tv.tv_sec * 1000L + tv.tv_usec / 1000L));
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm *tm_info;

    if (!buffer || buffer_size == 0)
        return (NULL);
    now = time(NULL);
    tm_info = localtime(&now);
    if (!tm_info)
        return (NULL);
    if (strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info) == 0)
        return (NULL);
    return (buffer);
}
