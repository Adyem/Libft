#include "../../Basic/basic.hpp"
#include "utils.hpp"

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
        result += ft_atoi(s, ft_nullptr);
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();

    print_comparison("atoi", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result)
        return (1);
    return (0);
}

