#ifndef TIME_HPP
# define TIME_HPP

#include <cstddef>
#include <ctime>
#include <chrono>
#include "../CPP_class/class_string_class.hpp"

typedef long t_time;

typedef struct s_monotonic_time_point
{
    long long milliseconds;
}   t_monotonic_time_point;

typedef struct s_duration_milliseconds
{
    long long milliseconds;
}   t_duration_milliseconds;

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
t_monotonic_time_point   time_monotonic_point_now(void);
t_monotonic_time_point   time_monotonic_point_add_ms(t_monotonic_time_point time_point, long long milliseconds);
long long   time_monotonic_point_diff_ms(t_monotonic_time_point start_point, t_monotonic_time_point end_point);
int     time_monotonic_point_compare(t_monotonic_time_point first_point, t_monotonic_time_point second_point);
t_duration_milliseconds  time_duration_ms_create(long long milliseconds);
void    time_local(t_time time_value, t_time_info *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);
size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info);
ft_string    time_format_iso8601(t_time time_value);
bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output, bool interpret_as_utc);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output);

typedef std::chrono::system_clock::time_point (*t_time_clock_now_hook)(void);

void    time_set_clock_now_hook(t_time_clock_now_hook hook);
void    time_reset_clock_now_hook(void);
#endif
