#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_cma_set_alloc_limit_resets_errno,
        "cma_set_alloc_limit sets ft_errno to success when locking")
{
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    cma_set_alloc_limit(64);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_limit_blocks_large_allocations_with_lock,
        "allocation limit rejects oversize requests when thread safety is enabled")
{
    void    *allocation;

    cma_set_thread_safety(true);
    cma_set_alloc_limit(16);
    ft_errno = FT_ERR_SUCCESSS;
    allocation = cma_malloc(32);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
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
    ft_errno = FT_ERR_SUCCESSS;
    allowed_allocation = cma_malloc(32);
    if (allowed_allocation == ft_nullptr)
    {
        cma_set_thread_safety(true);
        cma_set_alloc_limit(0);
        return (0);
    }
    blocked_allocation = cma_malloc(96);
    FT_ASSERT_EQ(ft_nullptr, blocked_allocation);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    cma_free(allowed_allocation);
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    return (1);
}
