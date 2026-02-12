#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cstring>
#include <vector>

int test_efficiency_memchr(void)
{
    const size_t iterations = 100000;
    std::vector<char> buf(4096, 'a');
    buf.back() = 'b';
    volatile const void *result = nullptr;
    auto std_memchr = static_cast<const void *(*)(const void *, int, size_t)>(std::memchr);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(buf.data());
        result = std_memchr(buf.data(), 'b', buf.size());
        prevent_optimization(const_cast<void*>(result));
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization(buf.data());
        result = ft_memchr(buf.data(), 'b', buf.size());
        prevent_optimization(const_cast<void*>(result));
    }
    auto end_ft = clock_type::now();
    print_comparison("memchr", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

