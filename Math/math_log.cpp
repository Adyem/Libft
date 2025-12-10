#include "math.hpp"
#include "../Errno/errno.hpp"

double math_log(double value)
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;
    unsigned long long exponent_bits;
    unsigned long long mantissa_bits;
    int    exponent;
    double mantissa;
    double mantissa_offset;
    double term;
    double result;
    double sign;
    int    iteration;

    if (value <= 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (math_nan());
    }
    converter.double_value = value;
    exponent_bits = (converter.bit_pattern >> 52) & 0x7ffULL;
    mantissa_bits = converter.bit_pattern & 0x000fffffffffffffULL;
    if (exponent_bits == 0)
    {
        int shift_count;

        shift_count = 0;
        while ((mantissa_bits & 0x0010000000000000ULL) == 0)
        {
            mantissa_bits <<= 1;
            shift_count += 1;
        }
        mantissa_bits &= 0x000fffffffffffffULL;
        exponent = -1022 - shift_count;
        converter.bit_pattern = mantissa_bits | 0x3ff0000000000000ULL;
    }
    else
    {
        exponent = static_cast<int>(exponent_bits) - 1023;
        converter.bit_pattern = mantissa_bits | 0x3ff0000000000000ULL;
    }
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
    ft_errno = FT_ER_SUCCESSS;
    return (exponent * 0.69314718055994530942 + result);
}
