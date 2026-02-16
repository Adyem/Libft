#include "time.hpp"
#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <climits>
#include <cstdint>

bool    time_high_resolution_now(t_high_resolution_time_point *time_point)
{
    int64_t     nanoseconds;
    int         error_code;

    if (!time_point)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (cmp_high_resolution_time(&nanoseconds) != 0)
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_TERMINATED;
        (void)(error_code);
        return (false);
    }
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)(error_code);
        return (false);
    }
    time_point->nanoseconds = static_cast<long long>(nanoseconds);
    (void)(FT_ERR_SUCCESS);
    return (true);
}

long long   time_high_resolution_diff_ns(t_high_resolution_time_point start_point, t_high_resolution_time_point end_point)
{
    __int128 diff_nanoseconds;

    diff_nanoseconds = static_cast<__int128>(end_point.nanoseconds);
    diff_nanoseconds -= static_cast<__int128>(start_point.nanoseconds);
    if (diff_nanoseconds > static_cast<__int128>(LLONG_MAX))
        return (LLONG_MAX);
    if (diff_nanoseconds < static_cast<__int128>(LLONG_MIN))
        return (LLONG_MIN);
    return (static_cast<long long>(diff_nanoseconds));
}

double  time_high_resolution_diff_seconds(t_high_resolution_time_point start_point, t_high_resolution_time_point end_point)
{
    long long   diff_nanoseconds;

    diff_nanoseconds = time_high_resolution_diff_ns(start_point, end_point);
    return (static_cast<double>(diff_nanoseconds) / 1000000000.0);
}
