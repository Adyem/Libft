#include "math.hpp"
#include "../Libft/limits.hpp"

int math_abs(int number)
{
    if (number < 0)
    {
        if (number == FT_INT_MIN)
            return (FT_INT_MAX);
        return (-number);
    }
    return (number);
}

long math_abs(long number)
{
    if (number < 0)
    {
        if (number == FT_LONG_MIN)
            return (FT_LONG_MAX);
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
