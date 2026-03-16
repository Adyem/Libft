#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include <ctime>
#include <cstdio>
#include <limits>

static ft_string time_format_failure(int32_t error_code)
{
    ft_string failure;

    (void)(error_code);
    return (failure);
}

static pt_mutex *g_time_format_gmtime_mutex = ft_nullptr;

static int32_t time_format_lock_gmtime_mutex(ft_bool *lock_acquired)
{
    pt_mutex *mutex_pointer;
    int32_t initialise_error;
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (g_time_format_gmtime_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        initialise_error = mutex_pointer->initialize();
        if (initialise_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (initialise_error);
        }
        g_time_format_gmtime_mutex = mutex_pointer;
    }
    lock_error = pt_mutex_lock_if_not_null(g_time_format_gmtime_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

static void time_format_unlock_gmtime_mutex(ft_bool lock_acquired)
{
    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_mutex_unlock_if_not_null(g_time_format_gmtime_mutex);
    return ;
}

ft_string    time_format_iso8601(t_time time_value)
{
    std::time_t standard_time;
    std::tm time_storage;
    std::tm *time_pointer;
    char buffer[21];
    ft_string formatted;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t strftime_result;

    standard_time = time_value;
    lock_acquired = FT_FALSE;
    lock_error = time_format_lock_gmtime_mutex(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (time_format_failure(lock_error));
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        time_format_unlock_gmtime_mutex(lock_acquired);
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    time_format_unlock_gmtime_mutex(lock_acquired);
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage);
    if (strftime_result == 0)
    {
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    if (formatted.initialize(buffer) != FT_ERR_SUCCESS)
        return (time_format_failure(ft_string::get_error()));
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}

ft_string    time_format_iso8601_with_offset(t_time time_value, int32_t offset_minutes)
{
    std::tm *time_pointer;
    std::tm time_storage;
    char buffer[20];
    char offset_buffer[7];
    ft_string formatted;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t strftime_result;
    int64_t offset_seconds;
    int64_t adjusted_epoch;
    std::time_t adjusted_time;
    char sign_character;
    int32_t absolute_offset;
    int32_t offset_hours;
    int32_t offset_minutes_part;
    int32_t snprintf_result;

    offset_seconds = static_cast<int64_t>(offset_minutes) * 60LL;
    adjusted_epoch = static_cast<int64_t>(time_value) + offset_seconds;
    if (adjusted_epoch > std::numeric_limits<std::time_t>::max())
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    if (adjusted_epoch < std::numeric_limits<std::time_t>::min())
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    adjusted_time = adjusted_epoch;
    lock_acquired = FT_FALSE;
    lock_error = time_format_lock_gmtime_mutex(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (time_format_failure(lock_error));
    time_pointer = std::gmtime(&adjusted_time);
    if (!time_pointer)
    {
        time_format_unlock_gmtime_mutex(lock_acquired);
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    time_format_unlock_gmtime_mutex(lock_acquired);
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
    if (snprintf_result < 0 || snprintf_result >= static_cast<int32_t>(sizeof(offset_buffer)))
        return (time_format_failure(FT_ERR_INTERNAL));
    if (formatted.initialize(buffer) != FT_ERR_SUCCESS)
        return (time_format_failure(ft_string::get_error()));
    formatted += offset_buffer;
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}
