#include "../../Libft/libft.hpp"
#include "utils.hpp"

#include <cstdlib>

int test_efficiency_atol(void)
{
    const size_t iterations = 100000;
    const char *s = "123456789";
    volatile long result = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)s);
        result += std::atol(s);
        prevent_optimization((void*)&result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)s);
        result += ft_atol(s);
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();

    print_comparison("atol", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result)
        return (1);
    return (0);
}

