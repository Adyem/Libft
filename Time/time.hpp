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

typedef struct s_high_resolution_time_point
{
    long long nanoseconds;
}   t_high_resolution_time_point;

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

struct event_loop;

typedef struct s_time_async_sleep
{
    t_monotonic_time_point deadline;
    bool completed;
}   t_time_async_sleep;

typedef struct s_time_benchmark
{
    size_t  sample_count;
    double  rolling_mean_ms;
    double  rolling_m2_ms;
    double  minimum_ms;
    double  maximum_ms;
    int     error_code;
}   t_time_benchmark;

typedef struct s_time_benchmark_snapshot
{
    size_t  sample_count;
    double  average_ms;
    double  jitter_ms;
    double  minimum_ms;
    double  maximum_ms;
}   t_time_benchmark_snapshot;

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
ft_string    time_format_iso8601_with_offset(t_time time_value, int offset_minutes);
bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output, bool interpret_as_utc);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output);
bool    time_high_resolution_now(t_high_resolution_time_point *time_point);
long long   time_high_resolution_diff_ns(t_high_resolution_time_point start_point, t_high_resolution_time_point end_point);
double  time_high_resolution_diff_seconds(t_high_resolution_time_point start_point, t_high_resolution_time_point end_point);
bool    time_get_local_offset(t_time time_value, int *offset_minutes, bool *is_daylight_saving);
bool    time_convert_timezone(t_time time_value, int source_offset_minutes, int target_offset_minutes, t_time *converted_time);
bool    time_get_monotonic_wall_anchor(t_monotonic_time_point &anchor_monotonic,
            long long &anchor_wall_ms);
bool    time_monotonic_to_wall_ms(t_monotonic_time_point monotonic_point,
            t_monotonic_time_point anchor_monotonic, long long anchor_wall_ms,
            long long &out_wall_ms);
bool    time_wall_ms_to_monotonic(long long wall_time_ms,
            t_monotonic_time_point anchor_monotonic, long long anchor_wall_ms,
            t_monotonic_time_point &out_monotonic);

void    time_async_sleep_init(t_time_async_sleep *sleep_state, long long delay_milliseconds);
bool    time_async_sleep_is_complete(const t_time_async_sleep *sleep_state);
long long   time_async_sleep_remaining_ms(t_time_async_sleep *sleep_state);
int     time_async_sleep_poll(event_loop *loop, t_time_async_sleep *sleep_state);

typedef std::chrono::system_clock::time_point (*t_time_clock_now_hook)(void);

void    time_set_clock_now_hook(t_time_clock_now_hook hook);
void    time_reset_clock_now_hook(void);

void    time_benchmark_init(t_time_benchmark *benchmark);
void    time_benchmark_reset(t_time_benchmark *benchmark);
int     time_benchmark_add_sample(t_time_benchmark *benchmark, double duration_ms);
int     time_benchmark_add_duration(t_time_benchmark *benchmark,
            t_duration_milliseconds duration);
bool    time_benchmark_snapshot(const t_time_benchmark *benchmark,
            t_time_benchmark_snapshot *out_snapshot);
size_t  time_benchmark_get_sample_count(const t_time_benchmark *benchmark);
double  time_benchmark_get_average_ms(const t_time_benchmark *benchmark);
double  time_benchmark_get_jitter_ms(const t_time_benchmark *benchmark);
double  time_benchmark_get_minimum_ms(const t_time_benchmark *benchmark);
double  time_benchmark_get_maximum_ms(const t_time_benchmark *benchmark);
int     time_benchmark_get_error(const t_time_benchmark *benchmark);
const char  *time_benchmark_get_error_str(const t_time_benchmark *benchmark);

bool    time_trace_begin_session(const char *file_path);
bool    time_trace_end_session(void);
bool    time_trace_begin_event(const char *name, const char *category);
bool    time_trace_end_event(void);
bool    time_trace_instant_event(const char *name, const char *category);
#endif
