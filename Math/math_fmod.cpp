#include "math.hpp"
#include <cmath>
#include <limits>

static int math_is_infinite_internal(double number)
{
    union
    {
        double double_value;
        unsigned long long integer_value;
    } converter;

    unsigned long long exponent_bits;
    unsigned long long mantissa_bits;

    converter.double_value = number;
    exponent_bits = converter.integer_value & 0x7ff0000000000000ULL;
    mantissa_bits = converter.integer_value & 0x000fffffffffffffULL;
    if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits == 0)
        return (1);
    return (0);
}

double math_fmod(double value, double modulus)
{
    double remainder_value;

    if (math_isnan(value) || math_isnan(modulus))
        return (math_nan());
    if (math_is_infinite_internal(value) != 0)
        return (math_nan());
    if (math_fabs(modulus) <= std::numeric_limits<double>::denorm_min())
        return (math_nan());
    if (math_is_infinite_internal(modulus) != 0)
        return (value);
    remainder_value = std::fmod(value, modulus);
    if (math_fabs(remainder_value) <= std::numeric_limits<double>::denorm_min())
        remainder_value = value * 0.0;
    return (remainder_value);
}

