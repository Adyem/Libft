#include "../test_internal.hpp"
#include "../../Math/math.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

int test_math_isnan_nan(void)
{
    double value;

    value = math_nan();
    if (!math_isnan(value))
        return (0);
    return (1);
}

int test_math_isnan_numbers(void)
{
    if (math_isnan(0.0))
        return (0);
    if (math_isnan(42.0))
        return (0);
    return (1);
}
