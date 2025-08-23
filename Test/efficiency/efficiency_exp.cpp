#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cmath>

int test_efficiency_exp(void)
{
    const size_t iterations = 100000;
    volatile double result = 0.0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        double x = (i % 100) / 50.0;
        result += std::exp(x);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        double x = (i % 100) / 50.0;
        result += ft_exp(x);
    }
    auto end_ft = clock_type::now();

    print_comparison("exp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

