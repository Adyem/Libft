#include "../test_internal.hpp"
#include "../../Template/promise.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <future>

int test_efficiency_promise_set_get(void)
{
    const size_t iterations = 10000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&sum);
        std::promise<int> p;
        std::future<int> f = p.get_future();
        p.set_value(static_cast<int>(i));
        sum += f.get();
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&sum);
        ft_promise<int> p;
        p.set_value(static_cast<int>(i));
        sum += p.get();
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("promise set/get", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

