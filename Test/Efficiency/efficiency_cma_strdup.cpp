#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "utils.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

#include <cstring>
#include <cstdlib>
#include <string>

static char *test_efficiency_cma_strdup_allocate(const char *source_string)
{
    char    *allocation_pointer;
    size_t   length;

    length = std::strlen(source_string) + 1;
    allocation_pointer = static_cast<char *>(cma_malloc(length));
    if (allocation_pointer == NULL)
        return (NULL);
    ft_memcpy(allocation_pointer, source_string, length);
    return (allocation_pointer);
}

int test_efficiency_cma_strdup(void)
{
    const size_t iterations = 50000;
    std::string s(1000, 'x');
    char *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = ::strdup(s.c_str());
        prevent_optimization(p);
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = test_efficiency_cma_strdup_allocate(s.c_str());
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    print_comparison("cma_strdup", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}
