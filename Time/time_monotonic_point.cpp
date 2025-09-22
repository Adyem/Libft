#include "time.hpp"
#include <chrono>

t_monotonic_time_point   time_monotonic_point_now(void)
{
    t_monotonic_time_point time_point;
    std::chrono::steady_clock::time_point chrono_now;
    std::chrono::milliseconds elapsed_milliseconds;

    chrono_now = std::chrono::steady_clock::now();
    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(chrono_now.time_since_epoch());
    time_point.milliseconds = elapsed_milliseconds.count();
    return (time_point);
}

t_monotonic_time_point   time_monotonic_point_add_ms(t_monotonic_time_point time_point, long long milliseconds)
{
    t_monotonic_time_point result_point;

    result_point.milliseconds = time_point.milliseconds + milliseconds;
    return (result_point);
}

long long   time_monotonic_point_diff_ms(t_monotonic_time_point start_point, t_monotonic_time_point end_point)
{
    long long difference;

    difference = end_point.milliseconds - start_point.milliseconds;
    return (difference);
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
