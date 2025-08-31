#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strcmp(void)
{
    const size_t iterations = 500000;
    std::string s1 = "abcdefghijklmnopqrstuvwxyz";
    std::string s2 = "abcdefghijklmnopqrstuvwxyz";
    volatile int result = 0;
    auto std_strcmp = static_cast<int (*)(const char *, const char *)>(std::strcmp);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(s1.data());
        prevent_optimization(s2.data());
        result += std_strcmp(s1.c_str(), s2.c_str());
        prevent_optimization((void*)&result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(s1.data());
        prevent_optimization(s2.data());
        result += ft_strcmp(s1.c_str(), s2.c_str());
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();

    print_comparison("strcmp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (result == 0)
        return (1);
    return (0);
}

