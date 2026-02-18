#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
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
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
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
        {
            (void)(reported_error);
            return (false);
        }
    }
    utc_pointer = std::gmtime(&standard_time);
    if (!utc_pointer)
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
            {
                (void)(reported_error);
                return (false);
            }
        }
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    *utc_out = *utc_pointer;
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
        {
            (void)(reported_error);
            return (false);
        }
    }
    (void)(FT_ERR_SUCCESS);
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
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    standard_time = time_value;
    if (cmp_localtime(&standard_time, &local_time) != 0)
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        (void)(error_code);
        return (false);
    }
    if (!load_utc_time(standard_time, &utc_time))
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        (void)(error_code);
        return (false);
    }
    local_epoch = std::mktime(&local_time);
    if (local_epoch == static_cast<std::time_t>(-1))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    utc_epoch = std::mktime(&utc_time);
    if (utc_epoch == static_cast<std::time_t>(-1))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    difference_seconds = static_cast<long long>(local_epoch) - static_cast<long long>(utc_epoch);
    minute_difference = difference_seconds / 60;
    if (minute_difference > static_cast<long long>(std::numeric_limits<int>::max()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (minute_difference < static_cast<long long>(std::numeric_limits<int>::min()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
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
    (void)(FT_ERR_SUCCESS);
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
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    base_seconds = static_cast<long long>(time_value);
    source_offset_seconds = static_cast<long long>(source_offset_minutes) * 60LL;
    target_offset_seconds = static_cast<long long>(target_offset_minutes) * 60LL;
    utc_seconds = base_seconds - source_offset_seconds;
    converted_seconds = utc_seconds + target_offset_seconds;
    if (converted_seconds > static_cast<long long>(std::numeric_limits<t_time>::max()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (converted_seconds < static_cast<long long>(std::numeric_limits<t_time>::min()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    *converted_time = converted_seconds;
    (void)(FT_ERR_SUCCESS);
    return (true);
}
