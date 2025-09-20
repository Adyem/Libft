#include "math.hpp"

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
    double absolute_value;
    double absolute_modulus;
    double remainder_value;
    double current_multiple;
    double tolerance;
    double result_sign;

    if (math_isnan(value) || math_isnan(modulus))
        return (math_nan());
    if (math_is_infinite_internal(value) != 0)
        return (math_nan());
    if (modulus == 0.0)
        return (math_nan());
    if (math_is_infinite_internal(modulus) != 0)
        return (value);
    absolute_value = math_fabs(value);
    absolute_modulus = math_fabs(modulus);
    if (absolute_modulus == 0.0)
        return (math_nan());
    if (absolute_value < absolute_modulus)
        return (value);
    remainder_value = absolute_value;
    tolerance = 0.0000000000001;
    while (remainder_value >= absolute_modulus)
    {
        current_multiple = absolute_modulus;
        while ((current_multiple + current_multiple) <= remainder_value)
            current_multiple = current_multiple + current_multiple;
        remainder_value = remainder_value - current_multiple;
    }
    if (remainder_value < tolerance)
        remainder_value = 0.0;
    if (math_fabs(absolute_modulus - remainder_value) < tolerance)
        remainder_value = 0.0;
    result_sign = 1.0;
    if (value < 0.0)
        result_sign = -1.0;
    remainder_value = remainder_value * result_sign;
    return (remainder_value);
}

