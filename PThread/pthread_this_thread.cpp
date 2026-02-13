#include "pthread.hpp"
#include "../Time/time.hpp"
#include "../Errno/errno.hpp"
#include <climits>

static unsigned int ft_this_thread_get_sleep_chunk(long long remaining_milliseconds)
{
    unsigned int chunk_milliseconds;

    if (remaining_milliseconds > static_cast<long long>(UINT_MAX))
    {
        chunk_milliseconds = UINT_MAX;
        return (chunk_milliseconds);
    }
    if (remaining_milliseconds < 0)
    {
        chunk_milliseconds = 0;
        return (chunk_milliseconds);
    }
    chunk_milliseconds = static_cast<unsigned int>(remaining_milliseconds);
    return (chunk_milliseconds);
}

t_thread_id ft_this_thread_get_id()
{
    t_thread_id thread_identifier;

    thread_identifier.native_id = pt_thread_self();
    return (thread_identifier);
}

void ft_this_thread_sleep_for(t_duration_milliseconds duration)
{
    long long remaining_milliseconds;
    unsigned int chunk_milliseconds;

    remaining_milliseconds = duration.milliseconds;
    if (remaining_milliseconds <= 0)
        return ;
    while (remaining_milliseconds > 0)
    {
        chunk_milliseconds = ft_this_thread_get_sleep_chunk(remaining_milliseconds);
        if (chunk_milliseconds == 0)
            return ;
        pt_thread_sleep(chunk_milliseconds);
        if (remaining_milliseconds <= static_cast<long long>(chunk_milliseconds))
            return ;
        remaining_milliseconds -= static_cast<long long>(chunk_milliseconds);
    }
    return ;
}

void ft_this_thread_sleep_until(t_monotonic_time_point time_point)
{
    t_monotonic_time_point current_time;
    long long remaining_milliseconds;
    t_duration_milliseconds duration;

    current_time = time_monotonic_point_now();
    remaining_milliseconds = time_monotonic_point_diff_ms(current_time, time_point);
    if (remaining_milliseconds <= 0)
        return ;
    duration = time_duration_ms_create(remaining_milliseconds);
    ft_this_thread_sleep_for(duration);
    return ;
}

void ft_this_thread_yield()
{
    pt_thread_yield();
    return ;
}
