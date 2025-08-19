#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strlen(void)
{
    const size_t iterations = 100000;
    std::string s(1000, 'a');
    volatile size_t sink = 0;
    auto std_strlen = static_cast<size_t (*)(const char *)>(std::strlen);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink += std_strlen(s.c_str());
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink += ft_strlen(s.c_str());
    auto end_ft = clock_type::now();

    (void)sink;
    print_comparison("strlen", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

