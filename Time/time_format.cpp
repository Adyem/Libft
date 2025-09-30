#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <ctime>

t_time_format_gmtime_override_function g_time_format_gmtime_override = ft_nullptr;
t_time_format_strftime_override_function g_time_format_strftime_override = ft_nullptr;
t_time_format_mutex_override_function g_time_format_lock_override = ft_nullptr;
t_time_format_mutex_override_function g_time_format_unlock_override = ft_nullptr;

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
    t_time_format_mutex_override_function lock_override;
    t_time_format_mutex_override_function unlock_override;
    int mutex_result;
    size_t strftime_result;

    standard_time = static_cast<std::time_t>(time_value);
    lock_override = g_time_format_lock_override;
    if (lock_override)
        mutex_result = lock_override(&g_gmtime_mutex, THREAD_ID);
    else
        mutex_result = g_gmtime_mutex.lock(THREAD_ID);
    if (mutex_result != FT_SUCCESS)
    {
        if (!lock_override)
            return (time_format_failure(g_gmtime_mutex.get_error()));
        if (ft_errno == ER_SUCCESS)
        {
            if (mutex_result == -1)
                return (time_format_failure(FT_EINVAL));
            return (time_format_failure(mutex_result));
        }
        return (time_format_failure(ft_errno));
    }
    if (g_time_format_gmtime_override)
        time_pointer = g_time_format_gmtime_override(&standard_time);
    else
        time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        unlock_override = g_time_format_unlock_override;
        if (unlock_override)
            mutex_result = unlock_override(&g_gmtime_mutex, THREAD_ID);
        else
            mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
        if (mutex_result != FT_SUCCESS)
        {
            if (!unlock_override)
                return (time_format_failure(g_gmtime_mutex.get_error()));
            if (ft_errno == ER_SUCCESS)
            {
                if (mutex_result == -1)
                    return (time_format_failure(FT_EINVAL));
                return (time_format_failure(mutex_result));
            }
            return (time_format_failure(ft_errno));
        }
        return (time_format_failure(FT_EINVAL));
    }
    time_storage = *time_pointer;
    unlock_override = g_time_format_unlock_override;
    if (unlock_override)
        mutex_result = unlock_override(&g_gmtime_mutex, THREAD_ID);
    else
        mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
    if (mutex_result != FT_SUCCESS)
    {
        if (!unlock_override)
            return (time_format_failure(g_gmtime_mutex.get_error()));
        if (ft_errno == ER_SUCCESS)
        {
            if (mutex_result == -1)
                return (time_format_failure(FT_EINVAL));
            return (time_format_failure(mutex_result));
        }
        return (time_format_failure(ft_errno));
    }
    if (g_time_format_strftime_override)
        strftime_result = g_time_format_strftime_override(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage);
    else
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
