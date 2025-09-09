#include "../../Libft/libft.hpp"
#include "utils.hpp"

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
    {
        prevent_optimization(s.data());
        sink += std_strlen(s.c_str());
        prevent_optimization((void*)&sink);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(s.data());
        sink += ft_strlen(s.c_str());
        prevent_optimization((void*)&sink);
    }
    auto end_ft = clock_type::now();
    print_comparison("strlen", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

