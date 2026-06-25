#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "utils.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

#include <cstdlib>
#include <vector>

static void *test_efficiency_cma_memdup_allocate(const void *source_pointer,
    size_t size)
{
    void    *allocation_pointer;

    allocation_pointer = cma_malloc(size);
    if (allocation_pointer == NULL)
        return (NULL);
    ft_memcpy(allocation_pointer, source_pointer, size);
    return (allocation_pointer);
}

static void *test_efficiency_std_memdup_allocate(const void *source_pointer,
    size_t size)
{
    void    *allocation_pointer;

    allocation_pointer = std::malloc(size);
    if (allocation_pointer == NULL)
        return (NULL);
    std::memcpy(allocation_pointer, source_pointer, size);
    return (allocation_pointer);
}

int test_efficiency_cma_memdup(void)
{
    const size_t iterations = 50000;
    std::vector<char> data(1024, 'a');
    void *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = test_efficiency_std_memdup_allocate(data.data(), data.size());
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = test_efficiency_cma_memdup_allocate(data.data(), data.size());
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    print_comparison("cma_memdup", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}
