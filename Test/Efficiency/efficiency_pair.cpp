#include "../test_internal.hpp"
#include "../../Template/pair.hpp"
#include "../../Template/swap.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <utility>

int test_efficiency_pair_create_copy(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&sum);
        std::pair<int, int> p(static_cast<int>(i), static_cast<int>(i + 1));
        std::pair<int, int> q = p;
        sum += q.first + q.second;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&sum);
        Pair<int, int> p(static_cast<int>(i), static_cast<int>(i + 1));
        Pair<int, int> q = p;
        sum += q.key + q.value;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("pair create/copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_pair_move(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        std::pair<int, int> p(static_cast<int>(i), static_cast<int>(i + 1));
        std::pair<int, int> q(std::move(p));
        sum += q.first + q.second;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        Pair<int, int> p(static_cast<int>(i), static_cast<int>(i + 1));
        Pair<int, int> q(std::move(p));
        sum += q.key + q.value;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("pair move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_pair_swap(void)
{
    const size_t iterations = 100000;
    std::pair<int, int> std_a(1, 2), std_b(3, 4);
    Pair<int, int> ft_a(1, 2), ft_b(3, 4);
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&std_a);
        prevent_optimization((void*)&std_b);
        std::swap(std_a, std_b);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ft_a);
        prevent_optimization((void*)&ft_b);
        ft_swap(ft_a, ft_b);
    }
    auto end_ft = clock_type::now();

    sum += std_a.first + std_b.first + ft_a.key + ft_b.key;
    prevent_optimization((void*)&sum);

    print_comparison("pair swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

