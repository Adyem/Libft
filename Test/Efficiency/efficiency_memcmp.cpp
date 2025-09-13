#include "../../Libft/libft.hpp"
#include "utils.hpp"

#include <cstring>
#include <vector>

int test_efficiency_memcmp(void)
{
    const size_t iterations = 50000;
    std::vector<char> a(4096, 'a');
    std::vector<char> b(4096, 'a');
    b.back() = 'b';
    volatile int result = 0;
    auto std_memcmp = static_cast<int (*)(const void *, const void *, size_t)>(std::memcmp);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(a.data());
        prevent_optimization(b.data());
        result += std_memcmp(a.data(), b.data(), a.size());
        prevent_optimization((void*)&result);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(a.data());
        prevent_optimization(b.data());
        result += ft_memcmp(a.data(), b.data(), a.size());
        prevent_optimization((void*)&result);
    }
    auto end_ft = clock_type::now();
    print_comparison("memcmp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

