#include "math.hpp"
#include "../Basic/limits.hpp"

static unsigned long long get_absolute_difference(long long first_number, long long second_number)
{
    __int128 signed_difference;

    signed_difference = static_cast<__int128>(first_number);
    signed_difference -= static_cast<__int128>(second_number);
    if (signed_difference >= 0)
        return (static_cast<unsigned long long>(signed_difference));
    return (static_cast<unsigned long long>(-signed_difference));
}

int math_absdiff(int first_number, int second_number)
{
    unsigned long long magnitude;

    magnitude = get_absolute_difference(static_cast<long long>(first_number), static_cast<long long>(second_number));
    if (magnitude > static_cast<unsigned long long>(FT_INT32_MAX))
        return (FT_INT32_MAX);
    return (static_cast<int>(magnitude));
}

long math_absdiff(long first_number, long second_number)
{
    unsigned long long magnitude;

    magnitude = get_absolute_difference(static_cast<long long>(first_number), static_cast<long long>(second_number));
    if (magnitude > static_cast<unsigned long long>(FT_LLONG_MAX))
        return (static_cast<long>(FT_LLONG_MAX));
    return (static_cast<long>(magnitude));
}

long long math_absdiff(long long first_number, long long second_number)
{
    unsigned long long magnitude;

    magnitude = get_absolute_difference(first_number, second_number);
    if (magnitude > static_cast<unsigned long long>(FT_LLONG_MAX))
        return (FT_LLONG_MAX);
    return (static_cast<long long>(magnitude));
}

double math_absdiff(double first_number, double second_number)
{
    double difference;

    difference = first_number - second_number;
    return (math_fabs(difference));
}
