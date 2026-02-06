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
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = time_info_lock(out, &lock_acquired);
    error_code = ft_global_error_stack_drop_last_error();
    if (lock_error != 0 || error_code != FT_ERR_SUCCESSS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return ;
    }
    standard_time = static_cast<std::time_t>(time_value);
    if (cmp_localtime(&standard_time, &temporary) != 0)
    {
        time_info_unlock(out, lock_acquired);
        ft_global_error_stack_drop_last_error();
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return ;
    }
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        time_info_unlock(out, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
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
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
