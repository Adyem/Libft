#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <chrono>
#include <climits>

t_monotonic_time_point   time_monotonic_point_now(void)
{
    t_monotonic_time_point time_point;
    std::chrono::steady_clock::time_point chrono_now;
    std::chrono::milliseconds elapsed_milliseconds;

    chrono_now = std::chrono::steady_clock::now();
    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(chrono_now.time_since_epoch());
    time_point.mutex = ft_nullptr;
    time_point.thread_safe_enabled = false;
    time_point.milliseconds = elapsed_milliseconds.count();
    return (time_point);
}

t_monotonic_time_point   time_monotonic_point_add_ms(t_monotonic_time_point time_point, long long milliseconds)
{
    t_monotonic_time_point result_point;
    __int128 sum_milliseconds;

    result_point.mutex = ft_nullptr;
    result_point.thread_safe_enabled = false;
    sum_milliseconds = static_cast<__int128>(time_point.milliseconds);
    sum_milliseconds += static_cast<__int128>(milliseconds);
    if (sum_milliseconds > static_cast<__int128>(LLONG_MAX))
        result_point.milliseconds = LLONG_MAX;
    else if (sum_milliseconds < static_cast<__int128>(LLONG_MIN))
        result_point.milliseconds = LLONG_MIN;
    else
        result_point.milliseconds = static_cast<long long>(sum_milliseconds);
    return (result_point);
}

t_monotonic_time_point   time_monotonic_point_create(long long milliseconds)
{
    t_monotonic_time_point time_point;

    time_point.mutex = ft_nullptr;
    time_point.thread_safe_enabled = false;
    time_point.milliseconds = milliseconds;
    return (time_point);
}

long long   time_monotonic_point_diff_ms(t_monotonic_time_point start_point, t_monotonic_time_point end_point)
{
    __int128 diff_milliseconds;

    diff_milliseconds = static_cast<__int128>(end_point.milliseconds);
    diff_milliseconds -= static_cast<__int128>(start_point.milliseconds);
    if (diff_milliseconds > static_cast<__int128>(LLONG_MAX))
        return (LLONG_MAX);
    if (diff_milliseconds < static_cast<__int128>(LLONG_MIN))
        return (LLONG_MIN);
    return (static_cast<long long>(diff_milliseconds));
}

int time_monotonic_point_compare(t_monotonic_time_point first_point, t_monotonic_time_point second_point)
{
    if (first_point.milliseconds < second_point.milliseconds)
    {
        return (-1);
    }
    if (first_point.milliseconds > second_point.milliseconds)
    {
        return (1);
    }
    return (0);
}
