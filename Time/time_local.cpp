#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <ctime>

void    time_local(t_time time_value, t_time_info *out)
{
    std::time_t    standard_time;
    std::tm        temporary;
    bool           lock_acquired;
    int            lock_error;
    int            error_code;

    if (!out)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = time_info_lock(out, &lock_acquired);
    error_code = FT_ERR_SUCCESS;
    if (lock_error != 0 || error_code != FT_ERR_SUCCESS)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_STATE;
        (void)(error_code);
        return ;
    }
    standard_time = static_cast<std::time_t>(time_value);
    if (cmp_localtime(&standard_time, &temporary) != 0)
    {
        time_info_unlock(out, lock_acquired);
        (void)(FT_ERR_SUCCESS);
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        (void)(error_code);
        return ;
    }
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        time_info_unlock(out, lock_acquired);
        (void)(FT_ERR_SUCCESS);
        (void)(error_code);
        return ;
    }
    out->seconds = temporary.tm_sec;
    out->minutes = temporary.tm_min;
    out->hours = temporary.tm_hour;
    out->month_day = temporary.tm_mday;
    out->month = temporary.tm_mon;
    out->year = temporary.tm_year;
    out->week_day = temporary.tm_wday;
    out->year_day = temporary.tm_yday;
    out->is_daylight_saving = temporary.tm_isdst;
    time_info_unlock(out, lock_acquired);
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)(error_code);
        return ;
    }
    (void)(FT_ERR_SUCCESS);
    return ;
}
