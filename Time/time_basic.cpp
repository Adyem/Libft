#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>

long int ft_time_ms(void)
{
    struct timeval time_value;
    if (cmp_time_get_time_of_day(&time_value) != 0)
    {
        return (-1);
    }
    long milliseconds = time_value.tv_sec * 1000;
    milliseconds += time_value.tv_usec / 1000;
    return (milliseconds);
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return (ft_nullptr);
    t_time current_time = time_now();
    if (current_time == -1)
        return (ft_nullptr);
    t_time_info time_info;
    ft_bzero(&time_info, sizeof(time_info));
    time_local(current_time, &time_info);
    size_t formatted_length = time_strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info);
    if (formatted_length == 0)
        return (ft_nullptr);
    return (buffer);
}
