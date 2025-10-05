#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <ctime>

static ft_string time_format_failure(int error_code)
{
    ft_string failure;

    ft_errno = error_code;
    return (failure);
}

ft_string    time_format_iso8601(t_time time_value)
{
    std::time_t standard_time;
    std::tm time_storage;
    std::tm *time_pointer;
    char buffer[21];
    ft_string formatted;
    static pt_mutex g_gmtime_mutex;
    int mutex_result;
    size_t strftime_result;

    standard_time = static_cast<std::time_t>(time_value);
    mutex_result = g_gmtime_mutex.lock(THREAD_ID);
    if (mutex_result != FT_SUCCESS)
    {
        return (time_format_failure(g_gmtime_mutex.get_error()));
    }
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
        if (mutex_result != FT_SUCCESS)
            return (time_format_failure(g_gmtime_mutex.get_error()));
        return (time_format_failure(FT_EINVAL));
    }
    time_storage = *time_pointer;
    mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
    if (mutex_result != FT_SUCCESS)
        return (time_format_failure(g_gmtime_mutex.get_error()));
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage);
    if (strftime_result == 0)
    {
        if (ft_errno == ER_SUCCESS)
            return (time_format_failure(FT_EINVAL));
        return (time_format_failure(ft_errno));
    }
    formatted = ft_string(buffer);
    ft_errno = ER_SUCCESS;
    return (formatted);
}
