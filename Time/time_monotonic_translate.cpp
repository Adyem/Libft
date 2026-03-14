#include "time.hpp"
#include "../Errno/errno.hpp"
#include <climits>

ft_bool    time_get_monotonic_wall_anchor(t_monotonic_time_point &anchor_monotonic,
            int64_t &anchor_wall_ms)
{
    t_monotonic_time_point    before_sample;
    t_monotonic_time_point    after_sample;
    t_monotonic_time_point    midpoint_sample;
    int64_t                 midpoint_offset;
    int64_t                      wall_now_ms;
    int32_t                       error_code;

    before_sample = time_monotonic_point_now();
    wall_now_ms = time_now_ms();
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)(error_code);
        return (FT_FALSE);
    }
    after_sample = time_monotonic_point_now();
    midpoint_offset = time_monotonic_point_diff_ms(before_sample, after_sample);
    if (midpoint_offset == LLONG_MAX || midpoint_offset == LLONG_MIN)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    midpoint_offset /= 2;
    midpoint_sample = time_monotonic_point_add_ms(before_sample, midpoint_offset);
    if (time_monotonic_point_diff_ms(before_sample, midpoint_sample) != midpoint_offset)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    anchor_monotonic = midpoint_sample;
    anchor_wall_ms = static_cast<int64_t>(wall_now_ms);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_monotonic_to_wall_ms(t_monotonic_time_point monotonic_point,
            t_monotonic_time_point anchor_monotonic, int64_t anchor_wall_ms,
            int64_t &out_wall_ms)
{
    int64_t    delta_ms;
    __int128     wall_candidate;

    delta_ms = time_monotonic_point_diff_ms(anchor_monotonic, monotonic_point);
    if (delta_ms == LLONG_MAX || delta_ms == LLONG_MIN)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    wall_candidate = static_cast<__int128>(anchor_wall_ms);
    wall_candidate += static_cast<__int128>(delta_ms);
    if (wall_candidate > static_cast<__int128>(LLONG_MAX)
        || wall_candidate < static_cast<__int128>(LLONG_MIN))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    out_wall_ms = static_cast<int64_t>(wall_candidate);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_wall_ms_to_monotonic(int64_t wall_time_ms,
            t_monotonic_time_point anchor_monotonic, int64_t anchor_wall_ms,
            t_monotonic_time_point &out_monotonic)
{
    __int128                delta_ms;
    int64_t               bounded_delta;
    t_monotonic_time_point  translated_point;
    int64_t               verification_delta;

    delta_ms = static_cast<__int128>(wall_time_ms);
    delta_ms -= static_cast<__int128>(anchor_wall_ms);
    if (delta_ms > static_cast<__int128>(LLONG_MAX)
        || delta_ms < static_cast<__int128>(LLONG_MIN))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    bounded_delta = static_cast<int64_t>(delta_ms);
    translated_point = time_monotonic_point_add_ms(anchor_monotonic, bounded_delta);
    verification_delta = time_monotonic_point_diff_ms(anchor_monotonic,
            translated_point);
    if (verification_delta != bounded_delta)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    out_monotonic = translated_point;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
