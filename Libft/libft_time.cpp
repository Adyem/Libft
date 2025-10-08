#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include <cerrno>

int64_t ft_time_ms(void)
{
    struct timeval time_value;
    int64_t milliseconds;

    ft_errno = ER_SUCCESS;
    if (cmp_time_get_time_of_day(&time_value) != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (-1);
    }
    milliseconds = static_cast<int64_t>(time_value.tv_sec) * 1000;
    milliseconds += static_cast<int64_t>(time_value.tv_usec) / 1000;
    return (milliseconds);
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    t_time current_time;
    t_time_info time_info;
    size_t formatted_length;

    if (!buffer || buffer_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    current_time = time_now();
    if (current_time == static_cast<t_time>(-1) && ft_errno != ER_SUCCESS)
    {
        return (ft_nullptr);
    }
    time_local(current_time, &time_info);
    if (ft_errno != ER_SUCCESS)
    {
        return (ft_nullptr);
    }
    formatted_length = time_strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info);
    if (formatted_length == 0)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    return (buffer);
}
