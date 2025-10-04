#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

static ft_size_t align_16(ft_size_t size)
{
    return ((size + 15) & ~static_cast<ft_size_t>(15));
}

FT_TEST(test_cma_block_size_reports_allocation, "cma_block_size returns the aligned allocation size")
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(48);
    if (!allocation_pointer)
        return (0);
    reported_size = cma_block_size(allocation_pointer);
    expected_minimum_size = align_16(48);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_block_size_null_pointer_sets_einval, "cma_block_size treats null pointers as invalid queries")
{
    ft_size_t reported_size;

    ft_errno = ER_SUCCESS;
    reported_size = cma_block_size(ft_nullptr);
    FT_ASSERT_EQ(reported_size, 0);
    FT_ASSERT_EQ(ft_errno, FT_EINVAL);
    return (1);
}

FT_TEST(test_cma_checked_block_size_reports_success, "cma_checked_block_size returns the aligned allocation size on success")
{
    void *allocation_pointer;
    ft_size_t reported_size;
    int query_result;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(96);
    if (!allocation_pointer)
        return (0);
    reported_size = 0;
    ft_errno = FT_EALLOC;
    query_result = cma_checked_block_size(allocation_pointer, &reported_size);
    FT_ASSERT_EQ(query_result, 0);
    expected_minimum_size = align_16(96);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_checked_block_size_rejects_invalid_pointer, "cma_checked_block_size detects non-CMA pointers")
{
    int stack_local;
    ft_size_t reported_size;
    int query_result;

    reported_size = 123;
    ft_errno = ER_SUCCESS;
    query_result = cma_checked_block_size(&stack_local, &reported_size);
    FT_ASSERT_EQ(query_result, -1);
    FT_ASSERT_EQ(reported_size, 0);
    FT_ASSERT_EQ(ft_errno, CMA_INVALID_PTR);
    return (1);
}

FT_TEST(test_cma_checked_block_size_null_output_pointer, "cma_checked_block_size validates the size output parameter")
{
    void *allocation_pointer;
    int query_result;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(24);
    if (!allocation_pointer)
        return (0);
    ft_errno = ER_SUCCESS;
    query_result = cma_checked_block_size(allocation_pointer, ft_nullptr);
    FT_ASSERT_EQ(query_result, -1);
    FT_ASSERT_EQ(ft_errno, FT_EINVAL);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_reports_success, "cma_alloc_size mirrors cma_checked_block_size aligned size behavior")
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(128);
    if (!allocation_pointer)
        return (0);
    ft_errno = FT_EALLOC;
    reported_size = cma_alloc_size(allocation_pointer);
    expected_minimum_size = align_16(128);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_rejects_invalid_pointer, "cma_alloc_size reports errors for non-CMA pointers")
{
    int stack_local;
    ft_size_t reported_size;

    ft_errno = ER_SUCCESS;
    reported_size = cma_alloc_size(&stack_local);
    FT_ASSERT_EQ(reported_size, 0);
    FT_ASSERT_EQ(ft_errno, CMA_INVALID_PTR);
    return (1);
}

FT_TEST(test_cma_alloc_size_null_pointer_sets_einval, "cma_alloc_size treats null pointers as invalid queries")
{
    ft_size_t reported_size;

    ft_errno = ER_SUCCESS;
    reported_size = cma_alloc_size(ft_nullptr);
    FT_ASSERT_EQ(reported_size, 0);
    FT_ASSERT_EQ(ft_errno, FT_EINVAL);
    return (1);
}

FT_TEST(test_cma_alloc_size_reports_block_size, "cma_alloc_size returns the stored aligned allocation size")
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(64);
    if (!allocation_pointer)
        return (0);
    ft_errno = FT_EINVAL;
    reported_size = cma_alloc_size(allocation_pointer);
    expected_minimum_size = align_16(64);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_null_pointer_sets_errno, "cma_alloc_size sets FT_EINVAL for null pointers")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), cma_alloc_size(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_alloc_size_rejects_non_cma_pointer, "cma_alloc_size detects pointers outside the allocator")
{
    char *allocation_pointer;
    ft_size_t reported_size;

    cma_set_alloc_limit(0);
    allocation_pointer = static_cast<char *>(cma_malloc(32));
    if (!allocation_pointer)
        return (0);
    ft_errno = ER_SUCCESS;
    reported_size = cma_alloc_size(allocation_pointer + 1);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), reported_size);
    FT_ASSERT_EQ(CMA_INVALID_PTR, ft_errno);
    cma_free(allocation_pointer);
    return (1);
}
