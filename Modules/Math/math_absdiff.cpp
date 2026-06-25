#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

static uint64_t get_absolute_difference(int64_t first_number,
    int64_t second_number)
{
    __int128 signed_difference;

    signed_difference = static_cast<__int128>(first_number);
    signed_difference -= static_cast<__int128>(second_number);
    if (signed_difference >= 0)
        return (static_cast<uint64_t>(signed_difference));
    return (static_cast<uint64_t>(-signed_difference));
}

int32_t math_absdiff(int32_t first_number, int32_t second_number)
{
    uint64_t magnitude;

    magnitude = get_absolute_difference(static_cast<int64_t>(first_number),
            static_cast<int64_t>(second_number));
    if (magnitude > static_cast<uint64_t>(FT_INT32_MAX))
        return (FT_INT32_MAX);
    return (static_cast<int32_t>(magnitude));
}

int64_t math_absdiff(int64_t first_number, int64_t second_number)
{
    uint64_t magnitude;

    magnitude = get_absolute_difference(first_number, second_number);
    if (magnitude > static_cast<uint64_t>(FT_LLONG_MAX))
        return (FT_LLONG_MAX);
    return (static_cast<int64_t>(magnitude));
}

double math_absdiff(double first_number, double second_number)
{
    double difference;

    difference = first_number - second_number;
    return (math_fabs(difference));
}
