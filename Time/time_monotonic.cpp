#include "time.hpp"

long    time_monotonic(void)
{
    t_monotonic_time_point time_point;

    time_point = time_monotonic_point_now();
    return ((long)time_point.milliseconds);
}

