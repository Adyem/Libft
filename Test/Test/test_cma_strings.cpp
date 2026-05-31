#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_strings_placeholder)
{
    void *allocation_pointer;

    allocation_pointer = cma_malloc(16);
    if (allocation_pointer == ft_nullptr)
        return (0);
    cma_free(allocation_pointer);
    return (1);
}
