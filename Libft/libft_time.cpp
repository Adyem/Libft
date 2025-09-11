#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Time/time.hpp"
#include <sys/time.h>

long ft_time_ms(void)
{
    struct timeval time_value;

    if (gettimeofday(&time_value, ft_nullptr) != 0)
        return (-1);
    return (time_value.tv_sec * 1000L + time_value.tv_usec / 1000L);
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    t_time current_time;
    t_time_info time_info;

    if (!buffer || buffer_size == 0)
        return (ft_nullptr);
    current_time = time_now();
    time_local(current_time, &time_info);
    if (time_strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info) == 0)
        return (ft_nullptr);
    return (buffer);
}
