#include "math.hpp"
#include "../Errno/errno.hpp"

double math_log(double value)
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;
    int    exponent;
    double mantissa;
    double mantissa_offset;
    double term;
    double result;
    double sign;
    int    iteration;

    if (value <= 0.0)
    {
        ft_errno = FT_EINVAL;
        return (math_nan());
    }
    converter.double_value = value;
    exponent = static_cast<int>((converter.bit_pattern >> 52) & 0x7ff) - 1023;
    converter.bit_pattern = (converter.bit_pattern & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL;
    mantissa = converter.double_value;
    mantissa_offset = mantissa - 1.0;
    term = mantissa_offset;
    result = mantissa_offset;
    sign = -1.0;
    iteration = 2;
    while (math_fabs(term) > 0.0000000001)
    {
        term = term * mantissa_offset;
        result = result + sign * term / iteration;
        sign = -sign;
        iteration = iteration + 1;
    }
    ft_errno = ER_SUCCESS;
    return (exponent * 0.69314718055994530942 + result);
}
