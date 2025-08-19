#include "../../CMA/CMA.hpp"
#include "efficiency_utils.hpp"

#include <cstdlib>

int test_efficiency_cma_calloc(void)
{
    const size_t iterations = 100000;
    const size_t count = 64;
    const size_t size = 16;
    void *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = std::calloc(count, size);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = cma_calloc(count, size);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    cma_cleanup();
    print_comparison("cma_calloc", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

