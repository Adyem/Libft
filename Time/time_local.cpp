#include "time.hpp"
#include <ctime>

void    time_local(t_time time_value, t_time_info *out)
{
    std::time_t standard_time;
    std::tm *temporary;

    if (!out)
        return ;
    standard_time = static_cast<std::time_t>(time_value);
    temporary = std::localtime(&standard_time);
    if (!temporary)
        return ;
    out->seconds = temporary->tm_sec;
    out->minutes = temporary->tm_min;
    out->hours = temporary->tm_hour;
    out->month_day = temporary->tm_mday;
    out->month = temporary->tm_mon;
    out->year = temporary->tm_year;
    out->week_day = temporary->tm_wday;
    out->year_day = temporary->tm_yday;
    out->is_daylight_saving = temporary->tm_isdst;
    return ;
}

