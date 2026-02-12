#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cmath>

int test_efficiency_pow(void)
{
    const size_t iterations = 100000;
    volatile double result = 0.0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        double base = 1.0 + (i % 10) * 0.1;
        result += std::pow(base, 5.0);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        double base = 1.0 + (i % 10) * 0.1;
        result += math_pow(base, 5);
    }
    auto end_ft = clock_type::now();

    print_comparison("pow", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result)
        return (1);
    return (0);
}

