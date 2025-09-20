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

double math_acos(double dot)
{
    double pi_value;
    double lower_bound;
    double upper_bound;
    double middle_value;
    double cosine_middle;
    double tolerance;
    int    iteration_count;

    if (math_isnan(dot))
        return (math_nan());
    if (math_is_infinite_internal(dot) != 0)
        return (math_nan());
    tolerance = 0.0000000000001;
    if (dot > 1.0)
    {
        if (math_fabs(dot - 1.0) <= tolerance)
            dot = 1.0;
        else
            return (math_nan());
    }
    if (dot < -1.0)
    {
        if (math_fabs(dot + 1.0) <= tolerance)
            dot = -1.0;
        else
            return (math_nan());
    }
    pi_value = 3.14159265358979323846;
    if (dot == 1.0)
        return (0.0);
    if (dot == -1.0)
        return (pi_value);
    if (dot == 0.0)
        return (pi_value * 0.5);
    lower_bound = 0.0;
    upper_bound = pi_value;
    iteration_count = 0;
    while (iteration_count < 100 && math_fabs(upper_bound - lower_bound) > tolerance)
    {
        middle_value = (lower_bound + upper_bound) * 0.5;
        cosine_middle = math_cos(middle_value);
        if (math_fabs(cosine_middle - dot) <= tolerance)
            return (middle_value);
        if (cosine_middle > dot)
            lower_bound = middle_value;
        else
            upper_bound = middle_value;
        iteration_count = iteration_count + 1;
    }
    return ((lower_bound + upper_bound) * 0.5);
}

