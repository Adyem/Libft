#include "../../CMA/CMA.hpp"
#include "efficiency_utils.hpp"

#include <cstdlib>

int test_efficiency_cma_malloc(void)
{
    const size_t iterations = 100000;
    const size_t size = 256;
    void *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = std::malloc(size);
        ((volatile char*)p)[0] = 0;
        prevent_optimization(p);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = cma_malloc(size);
        ((volatile char*)p)[0] = 0;
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    cma_cleanup();
    print_comparison("cma_malloc", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

