#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <vector>

int test_efficiency_memcpy(void)
{
    const size_t iterations = 50000;
    std::vector<char> src(4096, 'a');
    std::vector<char> dst_std(4096);
    std::vector<char> dst_ft(4096);
    volatile void *sink = nullptr;
    auto std_memcpy = static_cast<void *(*)(void *, const void *, size_t)>(std::memcpy);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = std_memcpy(dst_std.data(), src.data(), src.size());
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = ft_memcpy(dst_ft.data(), src.data(), src.size());
    auto end_ft = clock_type::now();

    (void)sink;
    print_comparison("memcpy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

