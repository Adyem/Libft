#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_set_alloc_limit_updates_limit,
        "cma_set_alloc_limit updates allocator limits when locking")
{
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    cma_set_alloc_limit(64);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_limit_blocks_large_allocations_with_lock,
        "allocation limit rejects oversize requests when thread safety is enabled")
{
    void    *allocation;

    cma_set_thread_safety(true);
    cma_set_alloc_limit(16);
    allocation = cma_malloc(32);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_limit_blocks_large_allocations_without_lock,
        "allocation limit still applies when thread safety is disabled")
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
