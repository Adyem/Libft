#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static ft_size_t align_16(ft_size_t size)
{
    return ((size + 15) & ~15);
}

FT_TEST(test_cma_block_size_reports_allocation)
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(48);
    if (!allocation_pointer)
        return (0);
    reported_size = cma_block_size(allocation_pointer);
    expected_minimum_size = align_16(48);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_block_size_null_pointer_sets_einval)
{
    ft_size_t reported_size;

    reported_size = cma_block_size(ft_nullptr);
    FT_ASSERT_EQ(reported_size, 0);
    return (1);
}

FT_TEST(test_cma_checked_block_size_reports_success)
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
    query_result = cma_checked_block_size(allocation_pointer, &reported_size);
    FT_ASSERT_EQ(query_result, 0);
    expected_minimum_size = align_16(96);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_checked_block_size_rejects_invalid_pointer)
{
    int stack_local = 0;
    ft_size_t reported_size;
    int query_result;

    reported_size = 123;
    query_result = cma_checked_block_size(&stack_local, &reported_size);
    FT_ASSERT_EQ(query_result, FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(reported_size, 0);
    return (1);
}

FT_TEST(test_cma_checked_block_size_null_output_pointer)
{
    void *allocation_pointer;
    int query_result;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(24);
    if (!allocation_pointer)
        return (0);
    query_result = cma_checked_block_size(allocation_pointer, ft_nullptr);
    FT_ASSERT_EQ(query_result, FT_ERR_INVALID_ARGUMENT);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_reports_success)
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(128);
    if (!allocation_pointer)
        return (0);
    reported_size = cma_alloc_size(allocation_pointer);
    expected_minimum_size = align_16(128);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_rejects_invalid_pointer)
{
    int stack_local = 0;
    ft_size_t reported_size;

    reported_size = cma_alloc_size(&stack_local);
    FT_ASSERT_EQ(reported_size, 0);
    return (1);
}

FT_TEST(test_cma_alloc_size_null_pointer_sets_einval)
{
    ft_size_t reported_size;

    reported_size = cma_alloc_size(ft_nullptr);
    FT_ASSERT_EQ(reported_size, 0);
    return (1);
}

FT_TEST(test_cma_alloc_size_reports_block_size)
{
    void *allocation_pointer;
    ft_size_t reported_size;
    ft_size_t expected_minimum_size;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(64);
    if (!allocation_pointer)
        return (0);
    reported_size = cma_alloc_size(allocation_pointer);
    expected_minimum_size = align_16(64);
    FT_ASSERT(reported_size >= expected_minimum_size);
    FT_ASSERT((reported_size % 16) == 0);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_alloc_size_null_pointer_sets_errno)
{
    FT_ASSERT_EQ(0, cma_alloc_size(ft_nullptr));
    return (1);
}

FT_TEST(test_cma_alloc_size_rejects_non_cma_pointer)
{
    char *allocation_pointer;
    ft_size_t reported_size;

    cma_set_alloc_limit(0);
    allocation_pointer = static_cast<char *>(cma_malloc(32));
    if (!allocation_pointer)
        return (0);
    reported_size = cma_alloc_size(allocation_pointer + 1);
    FT_ASSERT_EQ(0, reported_size);
    cma_free(allocation_pointer);
    return (1);
}
