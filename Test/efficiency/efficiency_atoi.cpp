#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstdlib>

int test_efficiency_atoi(void)
{
    const size_t iterations = 100000;
    const char *s = "12345";
    volatile int result = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)s);
        result += std::atoi(s);
        prevent_optimization((void*)&result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)s);
        result += ft_atoi(s);
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();

    print_comparison("atoi", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

