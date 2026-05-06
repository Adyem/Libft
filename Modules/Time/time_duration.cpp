#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"

t_duration_milliseconds  time_duration_ms_create(int64_t milliseconds)
{
    t_duration_milliseconds duration;

    duration.mutex = ft_nullptr;
    duration.thread_safe_enabled = FT_FALSE;
    duration.milliseconds = milliseconds;
    return (duration);
}
