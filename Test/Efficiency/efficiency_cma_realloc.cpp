#include "../../CMA/CMA.hpp"
#include "utils.hpp"

#include <cstdlib>

int test_efficiency_cma_realloc(void)
{
    const size_t iterations = 10000;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t size = 32;
        void *p = std::malloc(size);
        p = std::realloc(p, size * 2);
        p = std::realloc(p, size * 4);
        prevent_optimization(p);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t size = 32;
        void *p = cma_malloc(size);
        p = cma_realloc(p, size * 2);
        p = cma_realloc(p, size * 4);
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    cma_cleanup();
    print_comparison("cma_realloc", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

