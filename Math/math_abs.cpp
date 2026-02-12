#include "math.hpp"
#include "../Basic/limits.hpp"

int math_abs(int number)
{
    if (number < 0)
    {
        if (number == FT_INT32_MIN)
            return (FT_INT32_MAX);
        return (-number);
    }
    return (number);
}

long math_abs(long number)
{
    if (number < 0)
    {
        if (number == static_cast<long>(FT_LLONG_MIN))
            return (static_cast<long>(FT_LLONG_MAX));
        return (-number);
    }
    return (number);
}

long long math_abs(long long number)
{
    if (number < 0)
    {
        if (number == FT_LLONG_MIN)
            return (FT_LLONG_MAX);
        return (-number);
    }
    return (number);
}
