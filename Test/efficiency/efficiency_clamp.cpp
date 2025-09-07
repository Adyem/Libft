#include "../../Math/math.hpp"
#include "efficiency_utils.hpp"

#include <algorithm>

int test_efficiency_clamp(void)
{
    const size_t iterations = 1000000;
    volatile int result = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std::clamp(static_cast<int>(i % 200) - 100, -50, 50);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += math_clamp(static_cast<int>(i % 200) - 100, -50, 50);
    auto end_ft = clock_type::now();

    print_comparison("clamp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result)
        return (1);
    return (0);
}

