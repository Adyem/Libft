#include "math.hpp"

int math_isnan(double number)
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
    if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits != 0)
        return (1);
    return (0);
}
