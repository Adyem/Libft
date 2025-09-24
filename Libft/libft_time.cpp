#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include <cerrno>
#include <sys/time.h>

int64_t ft_time_ms(void)
{
    struct timeval time_value;
    int64_t milliseconds;

    if (gettimeofday(&time_value, ft_nullptr) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
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
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    current_time = time_now();
    time_local(current_time, &time_info);
    formatted_length = time_strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info);
    if (formatted_length == 0)
    {
        ft_errno = FT_ERANGE;
        return (ft_nullptr);
    }
    return (buffer);
}
