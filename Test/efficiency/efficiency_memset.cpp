#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <vector>

int test_efficiency_memset(void)
{
    const size_t iterations = 50000;
    std::vector<char> buf_std(4096);
    std::vector<char> buf_ft(4096);
    volatile void *sink = nullptr;
    auto std_memset = static_cast<void *(*)(void *, int, size_t)>(std::memset);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(buf_std.data());
        sink = std_memset(buf_std.data(), 'a', buf_std.size());
        prevent_optimization(buf_std.data());
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(buf_ft.data());
        sink = ft_memset(buf_ft.data(), 'a', buf_ft.size());
        prevent_optimization(buf_ft.data());
    }
    auto end_ft = clock_type::now();
    (void)sink;
    print_comparison("memset", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

