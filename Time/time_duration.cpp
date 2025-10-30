#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"

t_duration_milliseconds  time_duration_ms_create(long long milliseconds)
{
    t_duration_milliseconds duration;

    duration.mutex = ft_nullptr;
    duration.thread_safe_enabled = false;
    duration.milliseconds = milliseconds;
    return (duration);
}
