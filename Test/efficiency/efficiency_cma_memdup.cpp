#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <vector>

int test_efficiency_cma_memdup(void)
{
    const size_t iterations = 50000;
    std::vector<char> data(1024, 'a');
    void *p;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = ft_memdup(data.data(), data.size());
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        p = cma_memdup(data.data(), data.size());
        prevent_optimization(p);
        cma_free(p);
    }
    auto end_ft = clock_type::now();

    cma_cleanup();
    print_comparison("cma_memdup", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

