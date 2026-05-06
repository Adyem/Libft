#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

int test_cma_checked_free_basic(void)
{
    void *memory_pointer;

    memory_pointer = cma_malloc(32);
    if (!memory_pointer)
        return (0);
    if (cma_checked_free(memory_pointer) != 0)
        return (0);
    return (1);
}

int test_cma_checked_free_offset(void)
{
    char *memory_pointer;

    memory_pointer = static_cast<char *>(cma_malloc(32));
    if (!memory_pointer)
        return (0);
    if (cma_checked_free(memory_pointer + 10) == 0)
        return (0);
    cma_free(memory_pointer);
    return (1);
}

int test_cma_checked_free_invalid(void)
{
    int local_variable;

    if (cma_checked_free(&local_variable) == 0)
        return (0);
    return (1);
}

FT_TEST(test_cma_core_checked_free_basic)
{
    FT_ASSERT_EQ(1, test_cma_checked_free_basic());
    return (1);
}

FT_TEST(test_cma_core_checked_free_offset)
{
    FT_ASSERT_EQ(1, test_cma_checked_free_offset());
    return (1);
}

FT_TEST(test_cma_core_checked_free_invalid)
{
    FT_ASSERT_EQ(1, test_cma_checked_free_invalid());
    return (1);
}
