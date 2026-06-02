#include "math.hpp"
#include "../Errno/errno.hpp"
#include <cfloat>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

static int32_t math_is_infinite_internal(double number)
{
    union
    {
        double      double_value;
        uint64_t    integer_value;
    }   converter;

    uint64_t exponent_bits;
    uint64_t mantissa_bits;

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
    int32_t    iteration_count;
    int32_t    max_iterations;

    if (math_isnan(number))
    {
        return (math_nan());
    }
    if (number < 0)
    {
        return (math_nan());
    }
    if (math_is_infinite_internal(number) != 0)
    {
        return (number);
    }
    if (math_fabs(number) <= DBL_MIN)
    {
        return (0.0);
    }
    guess = number;
    iteration_count = 0;
    max_iterations = 1000;
    while (iteration_count < max_iterations)
    {
        next_guess = 0.5 * (guess + number / guess);
        if (math_isnan(next_guess))
        {
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
        {
            return (next_guess);
        }
        guess = next_guess;
        iteration_count += 1;
    }
    return (math_nan());
}
