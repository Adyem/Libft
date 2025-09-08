#include "../../Template/template_pool.hpp"
#include "efficiency_utils.hpp"

#include "../../Template/template_vector.hpp"

int test_efficiency_pool_acquire_release(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    ft_vector<int*> std_ptrs;
    std_ptrs.reserve(iterations);
    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        int* p = new int(static_cast<int>(i));
        prevent_optimization((void*)p);
        sum += *p;
        std_ptrs.push_back(p);
    }
    for (size_t i = 0; i < iterations; ++i)
    {
        sum += *std_ptrs[i];
        delete std_ptrs[i];
    }
    auto end_std = clock_type::now();

    Pool<int> pool;
    pool.resize(iterations);
    ft_vector<typename Pool<int>::Object> objs;
    objs.reserve(iterations);
    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        typename Pool<int>::Object obj = pool.acquire(static_cast<int>(i));
        prevent_optimization((void*)obj.operator->());
        sum += *obj.operator->();
        objs.push_back(std::move(obj));
    }
    objs.clear();
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("pool acquire/release", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

