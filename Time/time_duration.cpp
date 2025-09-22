#include "time.hpp"

t_duration_milliseconds  time_duration_ms_create(long long milliseconds)
{
    t_duration_milliseconds duration;

    duration.milliseconds = milliseconds;
    return (duration);
}
