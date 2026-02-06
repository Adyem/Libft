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
    int error_code;

    thread_identifier.native_id = pt_thread_self();
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(error_code);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (thread_identifier);
}

void ft_this_thread_sleep_for(t_duration_milliseconds duration)
{
    long long remaining_milliseconds;
    unsigned int chunk_milliseconds;
    int error_code;

    remaining_milliseconds = duration.milliseconds;
    if (remaining_milliseconds <= 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    while (remaining_milliseconds > 0)
    {
        chunk_milliseconds = ft_this_thread_get_sleep_chunk(remaining_milliseconds);
        if (chunk_milliseconds == 0)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return ;
        }
        pt_thread_sleep(chunk_milliseconds);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return ;
        }
        if (remaining_milliseconds <= static_cast<long long>(chunk_milliseconds))
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return ;
        }
        remaining_milliseconds -= static_cast<long long>(chunk_milliseconds);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

void ft_this_thread_sleep_until(t_monotonic_time_point time_point)
{
    t_monotonic_time_point current_time;
    long long remaining_milliseconds;
    t_duration_milliseconds duration;
    int error_code;

    current_time = time_monotonic_point_now();
    remaining_milliseconds = time_monotonic_point_diff_ms(current_time, time_point);
    if (remaining_milliseconds <= 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    duration = time_duration_ms_create(remaining_milliseconds);
    ft_this_thread_sleep_for(duration);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

void ft_this_thread_yield()
{
    int error_code;

    pt_thread_yield();
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
