#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strchr(void)
{
    const size_t iterations = 100000;
    std::string s(1000, 'a');
    s[500] = 'b';
    volatile const char *result = nullptr;
    auto std_strchr = static_cast<const char *(*)(const char *, int)>(std::strchr);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(const_cast<char*>(s.data()));
        result = std_strchr(s.c_str(), 'b');
        prevent_optimization(const_cast<char*>(result));
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(const_cast<char*>(s.data()));
        result = ft_strchr(s.c_str(), 'b');
        prevent_optimization(const_cast<char*>(result));
    }
    auto end_ft = clock_type::now();
    print_comparison("strchr", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

