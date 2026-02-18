#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_strings_placeholder, "cma string helpers placeholder")
{
    void *allocation_pointer;

    allocation_pointer = cma_malloc(16);
    if (allocation_pointer == ft_nullptr)
        return (0);
    cma_free(allocation_pointer);
    return (1);
}
