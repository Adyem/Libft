#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "utils.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

#include <cstdlib>

static void *test_efficiency_cma_calloc_allocate(size_t count, size_t size)
{
    void    *allocation_pointer;
    size_t   total_size;

    total_size = count * size;
    allocation_pointer = cma_malloc(total_size);
    if (allocation_pointer == NULL)
        return (NULL);
    ft_memset(allocation_pointer, 0, total_size);
    return (allocation_pointer);
}

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
        ((volatile char*)p)[0] = 0;
        prevent_optimization(p);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = test_efficiency_cma_calloc_allocate(count, size);
        ((volatile char*)p)[0] = 0;
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    print_comparison("cma_calloc", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}
