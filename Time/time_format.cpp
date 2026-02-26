#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <ctime>
#include <cstdio>
#include <limits>

static ft_string time_format_failure(int error_code)
{
    ft_string failure;

    (void)(error_code);
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
    int mutex_error;
    size_t strftime_result;

    standard_time = time_value;
    mutex_result = g_gmtime_mutex.lock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_SUCCESS)
            return (time_format_failure(reported_error));
    }
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        mutex_result = g_gmtime_mutex.unlock();
        if ((&g_gmtime_mutex) == ft_nullptr)
            mutex_error = FT_ERR_SUCCESS;
        else
            mutex_error = FT_ERR_SUCCESS;
        {
            int reported_error;

            if (mutex_error != FT_ERR_SUCCESS)
                reported_error = mutex_error;
            else
                reported_error = mutex_result;

            if (reported_error != FT_SUCCESS)
                return (time_format_failure(reported_error));
        }
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    mutex_result = g_gmtime_mutex.unlock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_SUCCESS)
            return (time_format_failure(reported_error));
    }
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage);
    if (strftime_result == 0)
    {
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    if (formatted.initialize(buffer) != FT_ERR_SUCCESS)
        return (time_format_failure(ft_string::last_operation_error()));
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}

ft_string    time_format_iso8601_with_offset(t_time time_value, int offset_minutes)
{
    std::tm *time_pointer;
    std::tm time_storage;
    char buffer[20];
    char offset_buffer[7];
    ft_string formatted;
    static pt_mutex g_gmtime_mutex;
    int mutex_result;
    int mutex_error;
    size_t strftime_result;
    long long offset_seconds;
    long long adjusted_epoch;
    std::time_t adjusted_time;
    char sign_character;
    int absolute_offset;
    int offset_hours;
    int offset_minutes_part;
    int snprintf_result;

    offset_seconds = static_cast<long long>(offset_minutes) * 60LL;
    adjusted_epoch = static_cast<long long>(time_value) + offset_seconds;
    if (adjusted_epoch > static_cast<long long>(std::numeric_limits<std::time_t>::max()))
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    if (adjusted_epoch < static_cast<long long>(std::numeric_limits<std::time_t>::min()))
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    adjusted_time = adjusted_epoch;
    mutex_result = g_gmtime_mutex.lock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_SUCCESS)
            return (time_format_failure(reported_error));
    }
    time_pointer = std::gmtime(&adjusted_time);
    if (!time_pointer)
    {
        mutex_result = g_gmtime_mutex.unlock();
        if ((&g_gmtime_mutex) == ft_nullptr)
            mutex_error = FT_ERR_SUCCESS;
        else
            mutex_error = FT_ERR_SUCCESS;
        {
            int reported_error;

            if (mutex_error != FT_ERR_SUCCESS)
                reported_error = mutex_error;
            else
                reported_error = mutex_result;

            if (reported_error != FT_SUCCESS)
                return (time_format_failure(reported_error));
        }
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    mutex_result = g_gmtime_mutex.unlock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_SUCCESS)
            return (time_format_failure(reported_error));
    }
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &time_storage);
    if (strftime_result == 0)
    {
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    if (offset_minutes >= 0)
        sign_character = '+';
    else
        sign_character = '-';
    absolute_offset = offset_minutes;
    if (absolute_offset < 0)
        absolute_offset = -absolute_offset;
    offset_hours = absolute_offset / 60;
    offset_minutes_part = absolute_offset % 60;
    snprintf_result = std::snprintf(offset_buffer, sizeof(offset_buffer), "%c%02d:%02d",
            sign_character, offset_hours, offset_minutes_part);
    if (snprintf_result < 0 || snprintf_result >= static_cast<int>(sizeof(offset_buffer)))
        return (time_format_failure(FT_ERR_INTERNAL));
    if (formatted.initialize(buffer) != FT_ERR_SUCCESS)
        return (time_format_failure(ft_string::last_operation_error()));
    formatted += offset_buffer;
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}
