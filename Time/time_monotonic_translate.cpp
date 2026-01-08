#include "time.hpp"
#include "../Errno/errno.hpp"
#include <climits>

bool    time_get_monotonic_wall_anchor(t_monotonic_time_point &anchor_monotonic,
            long long &anchor_wall_ms)
{
    t_monotonic_time_point    before_sample;
    t_monotonic_time_point    after_sample;
    t_monotonic_time_point    midpoint_sample;
    long long                 midpoint_offset;
    long                      wall_now_ms;
    int                       error_code;

    before_sample = time_monotonic_point_now();
    wall_now_ms = time_now_ms();
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (false);
    }
    after_sample = time_monotonic_point_now();
    midpoint_offset = time_monotonic_point_diff_ms(before_sample, after_sample);
    if (midpoint_offset == LLONG_MAX || midpoint_offset == LLONG_MIN)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    midpoint_offset /= 2;
    midpoint_sample = time_monotonic_point_add_ms(before_sample, midpoint_offset);
    if (time_monotonic_point_diff_ms(before_sample, midpoint_sample) != midpoint_offset)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    anchor_monotonic = midpoint_sample;
    anchor_wall_ms = static_cast<long long>(wall_now_ms);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_monotonic_to_wall_ms(t_monotonic_time_point monotonic_point,
            t_monotonic_time_point anchor_monotonic, long long anchor_wall_ms,
            long long &out_wall_ms)
{
    long long    delta_ms;
    __int128     wall_candidate;

    delta_ms = time_monotonic_point_diff_ms(anchor_monotonic, monotonic_point);
    if (delta_ms == LLONG_MAX || delta_ms == LLONG_MIN)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    wall_candidate = static_cast<__int128>(anchor_wall_ms);
    wall_candidate += static_cast<__int128>(delta_ms);
    if (wall_candidate > static_cast<__int128>(LLONG_MAX)
        || wall_candidate < static_cast<__int128>(LLONG_MIN))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    out_wall_ms = static_cast<long long>(wall_candidate);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_wall_ms_to_monotonic(long long wall_time_ms,
            t_monotonic_time_point anchor_monotonic, long long anchor_wall_ms,
            t_monotonic_time_point &out_monotonic)
{
    __int128                delta_ms;
    long long               bounded_delta;
    t_monotonic_time_point  translated_point;
    long long               verification_delta;

    delta_ms = static_cast<__int128>(wall_time_ms);
    delta_ms -= static_cast<__int128>(anchor_wall_ms);
    if (delta_ms > static_cast<__int128>(LLONG_MAX)
        || delta_ms < static_cast<__int128>(LLONG_MIN))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    bounded_delta = static_cast<long long>(delta_ms);
    translated_point = time_monotonic_point_add_ms(anchor_monotonic, bounded_delta);
    verification_delta = time_monotonic_point_diff_ms(anchor_monotonic,
            translated_point);
    if (verification_delta != bounded_delta)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    out_monotonic = translated_point;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}
