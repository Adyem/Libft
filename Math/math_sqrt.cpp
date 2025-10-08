#include "math.hpp"
#include "../Errno/errno.hpp"

static int math_is_infinite_internal(double number)
{
    union
    {
        double              double_value;
        unsigned long long  integer_value;
    }   converter;

    unsigned long long exponent_bits;
    unsigned long long mantissa_bits;

    converter.double_value = number;
    exponent_bits = converter.integer_value & 0x7ff0000000000000ULL;
    mantissa_bits = converter.integer_value & 0x000fffffffffffffULL;
    if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits == 0)
        return (1);
    return (0);
}

double math_sqrt(double number)
{
    double guess;
    double next_guess;
    double difference;
    double tolerance;
    int    iteration_count;
    int    max_iterations;

    if (math_isnan(number))
    {
        ft_errno = FT_EINVAL;
        return (math_nan());
    }
    if (number < 0)
    {
        ft_errno = FT_EINVAL;
        return (math_nan());
    }
    if (math_is_infinite_internal(number) != 0)
    {
        ft_errno = ER_SUCCESS;
        return (number);
    }
    if (number == 0.0)
    {
        ft_errno = ER_SUCCESS;
        return (0.0);
    }
    ft_errno = ER_SUCCESS;
    guess = number;
    iteration_count = 0;
    max_iterations = 1000;
    while (iteration_count < max_iterations)
    {
        next_guess = 0.5 * (guess + number / guess);
        if (math_isnan(next_guess))
        {
            ft_errno = FT_ERANGE;
            return (math_nan());
        }
        difference = math_fabs(next_guess - guess);
        tolerance = 0.000001;
        if (math_fabs(next_guess) < 1.0)
        {
            tolerance = 0.000001 * math_fabs(next_guess);
            if (tolerance < 1e-12)
                tolerance = 1e-12;
        }
        if (difference < tolerance)
            return (next_guess);
        guess = next_guess;
        iteration_count += 1;
    }
    ft_errno = FT_ERANGE;
    return (math_nan());
}
