#ifndef TIME_HPP
# define TIME_HPP

#include <cstddef>

typedef long t_time;

typedef struct s_time_info
{
    int seconds;
    int minutes;
    int hours;
    int month_day;
    int month;
    int year;
    int week_day;
    int year_day;
    int is_daylight_saving;
}   t_time_info;

t_time  time_now(void);
long    time_now_ms(void);
long    time_monotonic(void);
void    time_local(t_time time_value, t_time_info *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);
size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info);

#endif
