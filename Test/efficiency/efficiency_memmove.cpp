#include "../../Libft/libft.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <vector>

int test_efficiency_memmove(void)
{
    const size_t iterations = 50000;
    std::vector<char> buf_std(4096, 'a');
    std::vector<char> buf_ft = buf_std;
    volatile void *sink = nullptr;
    auto std_memmove = static_cast<void *(*)(void *, const void *, size_t)>(std::memmove);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = std_memmove(buf_std.data() + 1, buf_std.data(), buf_std.size() - 1);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = ft_memmove(buf_ft.data() + 1, buf_ft.data(), buf_ft.size() - 1);
    auto end_ft = clock_type::now();

    (void)sink;
    print_comparison("memmove", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

