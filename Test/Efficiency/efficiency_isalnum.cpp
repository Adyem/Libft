#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

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
    if (result)
        return (1);
    return (0);
}

