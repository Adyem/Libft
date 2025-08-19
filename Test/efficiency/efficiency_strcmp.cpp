#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>

int test_efficiency_strcmp(void)
{
    const size_t iterations = 500000;
    const char *s1 = "abcdefghijklmnopqrstuvwxyz";
    const char *s2 = "abcdefghijklmnopqrstuvwxyz";
    volatile int result = 0;
    auto std_strcmp = static_cast<int (*)(const char *, const char *)>(std::strcmp);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std_strcmp(s1, s2);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_strcmp(s1, s2);
    auto end_ft = clock_type::now();

    print_comparison("strcmp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result == 0 ? 1 : 0);
}

