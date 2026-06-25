#include "math.hpp"
#include "../Errno/errno.hpp"
#include <cfloat>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_pow(double base_value, int32_t exponent)
{
    double      result;
    int64_t   exponent_value;

    result = 1.0;
    exponent_value = static_cast<int64_t>(exponent);
    if (exponent_value < 0)
    {
        if (math_fabs(base_value) <= DBL_MIN)
        {
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
