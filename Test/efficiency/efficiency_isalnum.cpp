#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cctype>

int test_efficiency_isalnum(void)
{
    const size_t iterations = 1000000;
    volatile int result = 0;
    auto std_isalnum = static_cast<int (*)(int)>(std::isalnum);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std_isalnum('A');
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_isalnum('A');
    auto end_ft = clock_type::now();

    print_comparison("isalnum", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

