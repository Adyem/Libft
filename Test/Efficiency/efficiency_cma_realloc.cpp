#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "utils.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cstdlib>

int test_efficiency_cma_realloc(void)
{
    const size_t iterations = 10000;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t size = 32;
        void *p = std::malloc(size);
        if (!p)
        {
            return (0);
        }
        void *std_temp = std::realloc(p, size * 2);
        if (!std_temp)
        {
            std::free(p);
            return (0);
        }
        p = std_temp;
        std_temp = std::realloc(p, size * 4);
        if (!std_temp)
        {
            std::free(p);
            return (0);
        }
        p = std_temp;
        prevent_optimization(p);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t size = 32;
        void *p = cma_malloc(size);
        if (!p)
        {
            return (0);
        }
        void *ft_temp = cma_realloc(p, size * 2);
        if (!ft_temp)
        {
            cma_free(p);
            return (0);
        }
        p = ft_temp;
        ft_temp = cma_realloc(p, size * 4);
        if (!ft_temp)
        {
            cma_free(p);
            return (0);
        }
        p = ft_temp;
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    print_comparison("cma_realloc", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

