#include "../Libft/libft.hpp"

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
    double percent = ft_time ? (double)std_time / ft_time * 100.0 : 0.0;
    printf("%s std: %lld us (100%%)\n", name, std_time);
    printf("%s ft : %lld us (%.2f%%)\n", name, ft_time, percent);
}

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

int test_efficiency_memset(void)
{
    const size_t iterations = 50000;
    std::vector<char> buf_std(4096);
    std::vector<char> buf_ft(4096);
    volatile void *sink = nullptr;
    auto std_memset = static_cast<void *(*)(void *, int, size_t)>(std::memset);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = std_memset(buf_std.data(), 'a', buf_std.size());
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        sink = ft_memset(buf_ft.data(), 'a', buf_ft.size());
    auto end_ft = clock_type::now();

    (void)sink;
    print_comparison("memset", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_strcmp(void)
{
    const size_t iterations = 500000;
    const char *s1 = "abcdefghijklmnopqrstuvwxyz";
    const char *s2 = "abcdefghijklmnopqrstuvwxyz";
    volatile int result = 0;
    auto std_strcmp = static_cast<int (*)(const char *, const char *)>(std::strcmp);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std_strcmp(s1, s2);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_strcmp(s1, s2);
    auto end_ft = clock_type::now();

    print_comparison("strcmp", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result == 0 ? 1 : 0);
}

int test_efficiency_isdigit(void)
{
    const size_t iterations = 1000000;
    volatile int result = 0;
    auto std_isdigit = static_cast<int (*)(int)>(std::isdigit);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += std_isdigit('5');
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
        result += ft_isdigit('5');
    auto end_ft = clock_type::now();

    print_comparison("isdigit", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (result ? 1 : 0);
}

