#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/lock_error_helpers.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <ctime>
#include <limits>

static bool load_utc_time(std::time_t standard_time, std::tm *utc_out)
{
    static pt_mutex g_gmtime_mutex;
    std::tm *utc_pointer;
    int mutex_result;
    int mutex_error;

    if (!utc_out)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    mutex_result = g_gmtime_mutex.lock(THREAD_ID);
    mutex_error = ft_mutex_pop_last_error(&g_gmtime_mutex);
    {
        int reported_error = mutex_error != FT_ERR_SUCCESSS ? mutex_error : mutex_result;

        if (reported_error != FT_SUCCESS)
        {
            ft_global_error_stack_push(reported_error);
            return (false);
        }
    }
    utc_pointer = std::gmtime(&standard_time);
    if (!utc_pointer)
    {
        mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
        mutex_error = ft_mutex_pop_last_error(&g_gmtime_mutex);
        {
            int reported_error = mutex_error != FT_ERR_SUCCESSS ? mutex_error : mutex_result;

            if (reported_error != FT_SUCCESS)
            {
                ft_global_error_stack_push(reported_error);
                return (false);
            }
        }
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    *utc_out = *utc_pointer;
    mutex_result = g_gmtime_mutex.unlock(THREAD_ID);
    mutex_error = ft_mutex_pop_last_error(&g_gmtime_mutex);
    {
        int reported_error = mutex_error != FT_ERR_SUCCESSS ? mutex_error : mutex_result;

        if (reported_error != FT_SUCCESS)
        {
            ft_global_error_stack_push(reported_error);
            return (false);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_get_local_offset(t_time time_value, int *offset_minutes, bool *is_daylight_saving)
{
    std::time_t standard_time;
    std::tm local_time;
    std::tm utc_time;
    std::time_t local_epoch;
    std::time_t utc_epoch;
    long long difference_seconds;
    long long minute_difference;
    int error_code;

    if (!offset_minutes && !is_daylight_saving)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    standard_time = static_cast<std::time_t>(time_value);
    if (cmp_localtime(&standard_time, &local_time) != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (false);
    }
    if (!load_utc_time(standard_time, &utc_time))
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (false);
    }
    local_epoch = std::mktime(&local_time);
    if (local_epoch == static_cast<std::time_t>(-1))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    utc_epoch = std::mktime(&utc_time);
    if (utc_epoch == static_cast<std::time_t>(-1))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    difference_seconds = static_cast<long long>(local_epoch) - static_cast<long long>(utc_epoch);
    minute_difference = difference_seconds / 60;
    if (minute_difference > static_cast<long long>(std::numeric_limits<int>::max()))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (minute_difference < static_cast<long long>(std::numeric_limits<int>::min()))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (offset_minutes)
        *offset_minutes = static_cast<int>(minute_difference);
    if (is_daylight_saving)
    {
        if (local_time.tm_isdst > 0)
            *is_daylight_saving = true;
        else
            *is_daylight_saving = false;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_convert_timezone(t_time time_value, int source_offset_minutes, int target_offset_minutes, t_time *converted_time)
{
    long long base_seconds;
    long long source_offset_seconds;
    long long target_offset_seconds;
    long long utc_seconds;
    long long converted_seconds;

    if (!converted_time)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    base_seconds = static_cast<long long>(time_value);
    source_offset_seconds = static_cast<long long>(source_offset_minutes) * 60LL;
    target_offset_seconds = static_cast<long long>(target_offset_minutes) * 60LL;
    utc_seconds = base_seconds - source_offset_seconds;
    converted_seconds = utc_seconds + target_offset_seconds;
    if (converted_seconds > static_cast<long long>(std::numeric_limits<t_time>::max()))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (converted_seconds < static_cast<long long>(std::numeric_limits<t_time>::min()))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    *converted_time = static_cast<t_time>(converted_seconds);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}
