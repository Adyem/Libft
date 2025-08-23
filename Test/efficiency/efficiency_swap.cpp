#include "../../Template/swap.hpp"
#include "efficiency_utils.hpp"

#include <utility>

struct Large
{
    long data[64];
};

int test_efficiency_swap_large(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;
    Large a;
    Large b;
    for (int i = 0; i < 64; ++i)
    {
        a.data[i] = i;
        b.data[i] = i + 1;
    }

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&a);
        prevent_optimization((void*)&b);
        std::swap(a, b);
        sum += a.data[0];
    }
    auto end_std = clock_type::now();

    for (int i = 0; i < 64; ++i)
    {
        a.data[i] = i;
        b.data[i] = i + 1;
    }

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&a);
        prevent_optimization((void*)&b);
        ft_swap(a, b);
        sum += a.data[0];
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("swap large struct", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

