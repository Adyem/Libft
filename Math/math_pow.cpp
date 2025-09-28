#include "math.hpp"
#include "../Errno/errno.hpp"
#include <cfloat>

double math_pow(double base_value, int exponent)
{
    double result;
    int    exponent_value;

    result = 1.0;
    exponent_value = exponent;
    ft_errno = ER_SUCCESS;
    if (exponent_value < 0)
    {
        if (math_fabs(base_value) <= DBL_MIN)
        {
            ft_errno = FT_EINVAL;
            return (math_nan());
        }
        base_value = 1.0 / base_value;
        exponent_value = -exponent_value;
    }
    while (exponent_value > 0)
    {
        if (exponent_value % 2 == 1)
            result *= base_value;
        base_value *= base_value;
        exponent_value /= 2;
    }
    return (result);
}
