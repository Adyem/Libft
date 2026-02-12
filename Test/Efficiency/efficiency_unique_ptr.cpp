#include "../test_internal.hpp"
#include "../../Template/unique_ptr.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <memory>
#include <utility>

int test_efficiency_unique_ptr_create(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        std::unique_ptr<int> up(new int(static_cast<int>(i)));
        prevent_optimization(up.get());
        sum += *up;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        ft_uniqueptr<int> up(new int(static_cast<int>(i)));
        prevent_optimization(up.get());
        sum += *up;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unique_ptr create", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_unique_ptr_move(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::unique_ptr<int> up(new int(42));
        for (size_t i = 0; i < iterations; ++i)
        {
            std::unique_ptr<int> mv = std::move(up);
            prevent_optimization(mv.get());
            sum += *mv;
            up = std::move(mv);
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_uniqueptr<int> up(new int(42));
        for (size_t i = 0; i < iterations; ++i)
        {
            ft_uniqueptr<int> mv = std::move(up);
            prevent_optimization(mv.get());
            sum += *mv;
            up = std::move(mv);
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unique_ptr move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

