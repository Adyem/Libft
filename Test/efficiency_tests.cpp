#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"

#include <cstring>
#include <chrono>
#include <string>
#include <vector>

using clock_type = std::chrono::high_resolution_clock;

static long long elapsed_us(clock_type::time_point start, clock_type::time_point end)
{
    return (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
}

static void print_comparison(const char *name, long long std_time, long long ft_time)
{
    double percent = (double)ft_time / std_time * 100.0;
    pf_printf("%s std: %lld us (100%%)\n", name, std_time);
    pf_printf("%s ft : %lld us (%.2f%%)\n", name, ft_time, percent);
}

int test_efficiency_strlen(void)
{
    const size_t iterations = 100000;
    std::string s(1000, 'a');

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        std::strlen(s.c_str());
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        ft_strlen(s.c_str());
    auto end_ft = clock_type::now();

    print_comparison("strlen", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_memcpy(void)
{
    const size_t iterations = 50000;
    std::vector<char> src(4096, 'a');
    std::vector<char> dst(4096);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        std::memcpy(dst.data(), src.data(), src.size());
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        ft_memcpy(dst.data(), src.data(), src.size());
    auto end_ft = clock_type::now();

    print_comparison("memcpy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_isdigit(void)
{
    const size_t iterations = 1000000;
    int result = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std::isdigit('5');
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_isdigit('5');
    auto end_ft = clock_type::now();

    print_comparison("isdigit", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

