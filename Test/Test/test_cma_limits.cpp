#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_set_alloc_limit_updates_limit)
{
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    cma_set_alloc_limit(64);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_limit_blocks_large_allocations_with_lock)
{
    void    *allocation;

    cma_set_thread_safety(true);
    cma_set_alloc_limit(16);
    allocation = cma_malloc(32);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_limit_blocks_large_allocations_without_lock)
{
    void    *allowed_allocation;
    void    *blocked_allocation;

    cma_set_thread_safety(false);
    cma_set_alloc_limit(64);
    allowed_allocation = cma_malloc(32);
    if (allowed_allocation == ft_nullptr)
    {
        cma_set_thread_safety(true);
        cma_set_alloc_limit(0);
        return (0);
    }
    blocked_allocation = cma_malloc(96);
    FT_ASSERT_EQ(ft_nullptr, blocked_allocation);
    cma_free(allowed_allocation);
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    return (1);
}
