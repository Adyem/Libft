#include "../../CMA/CMA.hpp"
#include "efficiency_utils.hpp"

#include <cstring>
#include <cstdlib>
#include <string>

int test_efficiency_cma_strdup(void)
{
    const size_t iterations = 50000;
    std::string s(1000, 'x');
    char *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = ::strdup(s.c_str());
        std::free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = cma_strdup(s.c_str());
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    cma_cleanup();
    print_comparison("cma_strdup", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

