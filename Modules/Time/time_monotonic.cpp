#include "time.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int64_t   time_monotonic(void)
{
    t_monotonic_time_point time_point;

    time_point = time_monotonic_point_now();
    return (time_point.milliseconds);
}

