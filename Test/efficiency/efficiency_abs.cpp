#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstdlib>

int test_efficiency_abs(void)
{
    const size_t iterations = 1000000;
    volatile int result = 0;
    auto std_abs = static_cast<int (*)(int)>(std::abs);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std_abs(-123);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_abs(-123);
    auto end_ft = clock_type::now();

    print_comparison("abs", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

