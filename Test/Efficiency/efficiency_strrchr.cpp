#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cstring>
#include <string>

int test_efficiency_strrchr(void)
{
    const size_t iterations = 100000;
    std::string s(1000, 'a');
    s.back() = 'b';
    volatile const char *result = nullptr;
    auto std_strrchr = static_cast<const char *(*)(const char *, int)>(std::strrchr);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(s.data());
        result = std_strrchr(s.c_str(), 'b');
        prevent_optimization(result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(s.data());
        result = ft_strrchr(s.c_str(), 'b');
        prevent_optimization(result);
    }
    auto end_ft = clock_type::now();

    print_comparison("strrchr", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}
