#include "math.hpp"
#include "../Errno/errno.hpp"
#include <cfloat>

double math_pow(double base_value, int exponent)
{
    double      result;
    long long   exponent_value;

    result = 1.0;
    exponent_value = static_cast<long long>(exponent);
    if (exponent_value < 0)
    {
        if (math_fabs(base_value) <= DBL_MIN)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
