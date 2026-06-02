#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <ctime>
#include <limits>
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_bool load_utc_time(std::time_t standard_time, std::tm *utc_out)
{
    static pt_mutex g_gmtime_mutex;
    std::tm *utc_pointer;
    int32_t mutex_result;
    int32_t mutex_error;

    if (!utc_out)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    mutex_result = g_gmtime_mutex.lock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int32_t reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_ERR_SUCCESS)
        {
            (void)(reported_error);
            return (FT_FALSE);
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
            int32_t reported_error;

            if (mutex_error != FT_ERR_SUCCESS)
                reported_error = mutex_error;
            else
                reported_error = mutex_result;

            if (reported_error != FT_ERR_SUCCESS)
            {
                (void)(reported_error);
                return (FT_FALSE);
            }
        }
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    *utc_out = *utc_pointer;
    mutex_result = g_gmtime_mutex.unlock();
    if ((&g_gmtime_mutex) == ft_nullptr)
        mutex_error = FT_ERR_SUCCESS;
    else
        mutex_error = FT_ERR_SUCCESS;
    {
        int32_t reported_error;

        if (mutex_error != FT_ERR_SUCCESS)
            reported_error = mutex_error;
        else
            reported_error = mutex_result;

        if (reported_error != FT_ERR_SUCCESS)
        {
            (void)(reported_error);
            return (FT_FALSE);
        }
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_get_local_offset(t_time time_value, int32_t *offset_minutes, ft_bool *is_daylight_saving)
{
    std::time_t standard_time;
    std::tm local_time;
    std::tm utc_time;
    std::time_t local_epoch;
    std::time_t utc_epoch;
    int64_t difference_seconds;
    int64_t minute_difference;
    int32_t error_code;

    if (!offset_minutes && !is_daylight_saving)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    standard_time = time_value;
    if (cmp_localtime(&standard_time, &local_time) != 0)
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        (void)(error_code);
        return (FT_FALSE);
    }
    if (!load_utc_time(standard_time, &utc_time))
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        (void)(error_code);
        return (FT_FALSE);
    }
    local_epoch = std::mktime(&local_time);
    if (local_epoch == static_cast<std::time_t>(-1))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    utc_epoch = std::mktime(&utc_time);
    if (utc_epoch == static_cast<std::time_t>(-1))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    difference_seconds = local_epoch - utc_epoch;
    minute_difference = difference_seconds / 60;
    if (minute_difference > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (minute_difference < static_cast<int64_t>(std::numeric_limits<int32_t>::min()))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (offset_minutes)
        *offset_minutes = static_cast<int32_t>(minute_difference);
    if (is_daylight_saving)
    {
        if (local_time.tm_isdst > 0)
            *is_daylight_saving = FT_TRUE;
        else
            *is_daylight_saving = FT_FALSE;
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_convert_timezone(t_time time_value, int32_t source_offset_minutes, int32_t target_offset_minutes, t_time *converted_time)
{
    int64_t base_seconds;
    int64_t source_offset_seconds;
    int64_t target_offset_seconds;
    int64_t utc_seconds;
    int64_t converted_seconds;

    if (!converted_time)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    base_seconds = time_value;
    source_offset_seconds = static_cast<int64_t>(source_offset_minutes) * 60LL;
    target_offset_seconds = static_cast<int64_t>(target_offset_minutes) * 60LL;
    utc_seconds = base_seconds - source_offset_seconds;
    converted_seconds = utc_seconds + target_offset_seconds;
    if (converted_seconds > std::numeric_limits<t_time>::max())
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (converted_seconds < std::numeric_limits<t_time>::min())
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    *converted_time = converted_seconds;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
