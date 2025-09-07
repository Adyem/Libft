#include "../../Math/math.hpp"
#include "efficiency_utils.hpp"

#include <cmath>

int test_efficiency_sqrt(void)
{
    const size_t iterations = 100000;
    volatile double result = 0.0;

    auto start_std = clock_type::now();
    for (size_t i = 1; i <= iterations; ++i)
        result += std::sqrt(static_cast<double>(i));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 1; i <= iterations; ++i)
        result += ft_sqrt(static_cast<double>(i));
    auto end_ft = clock_type::now();

    print_comparison("sqrt", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result)
        return (1);
    return (0);
}

