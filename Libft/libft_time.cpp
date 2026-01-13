#include "libft_config.hpp"

#if LIBFT_ENABLE_TIME_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include <cerrno>

static int64_t report_time_error(int error_code, int64_t return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

static char *report_time_string_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return (ft_nullptr);
}

int64_t ft_time_ms(void)
{
    struct timeval time_value;
    int64_t milliseconds;
    int error_code;

    if (cmp_time_get_time_of_day(&time_value) != 0)
    {
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = FT_ERR_TERMINATED;
        return (report_time_error(error_code, -1));
    }
    milliseconds = time_value.tv_sec * 1000;
    milliseconds += time_value.tv_usec / 1000;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (milliseconds);
}

char *ft_time_format(char *buffer, size_t buffer_size)
{
    t_time current_time;
    t_time_info time_info;
    size_t formatted_length;
    int error_code;

    if (!buffer || buffer_size == 0)
        return (report_time_string_error(FT_ERR_INVALID_ARGUMENT));
    ft_bzero(&time_info, sizeof(time_info));
    current_time = time_now();
    error_code = ft_global_error_stack_pop_newest();
    if (current_time == static_cast<t_time>(-1) || error_code != FT_ERR_SUCCESSS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_TERMINATED;
        return (report_time_string_error(error_code));
    }
    time_local(current_time, &time_info);
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        return (report_time_string_error(error_code));
    formatted_length = time_strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info);
    error_code = ft_global_error_stack_pop_newest();
    if (formatted_length == 0 || error_code != FT_ERR_SUCCESSS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_OUT_OF_RANGE;
        return (report_time_string_error(error_code));
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (buffer);
}
#endif
