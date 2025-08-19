#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strncmp(void)
{
    const size_t iterations = 100000;
    std::string a(1000, 'a');
    std::string b(1000, 'a');
    b.back() = 'b';
    volatile int result = 0;
    auto std_strncmp = static_cast<int (*)(const char *, const char *, size_t)>(std::strncmp);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(a.data());
        prevent_optimization(b.data());
        result += std_strncmp(a.c_str(), b.c_str(), a.size());
        prevent_optimization((void*)&result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(a.data());
        prevent_optimization(b.data());
        result += ft_strncmp(a.c_str(), b.c_str(), a.size());
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();
    print_comparison("strncmp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

