#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>

FT_TEST(test_cma_calloc_overflow_guard, "cma_calloc rejects overflowed sizes")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocated_pointer;

    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocated_pointer = cma_calloc(SIZE_MAX, 2);
    int allocation_errno = ft_errno;
    cma_get_stats(&allocation_count_after, ft_nullptr);
    ft_errno = allocation_errno;
    FT_ASSERT(allocated_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    FT_ASSERT_EQ(allocation_errno, FT_EINVAL);
    return (1);
}

FT_TEST(test_cma_malloc_zero_size_allocates, "cma_malloc returns a block for zero-size requests")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocation_pointer;
    ft_size_t expected_allocation_count;

    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(0);
    if (!allocation_pointer)
        return (0);
    cma_get_stats(&allocation_count_after, ft_nullptr);
    cma_free(allocation_pointer);
    expected_allocation_count = allocation_count_before + 1;
    FT_ASSERT_EQ(expected_allocation_count, allocation_count_after);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    return (1);
}

FT_TEST(test_cma_realloc_failure_preserves_original_buffer, "cma_realloc keeps the original buffer when growth fails")
{
    char *original_buffer;
    void *realloc_result;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char *>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'X', 16);
    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    realloc_result = cma_realloc(original_buffer, 32);
    int realloc_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, realloc_result);
    FT_ASSERT_EQ(realloc_errno, FT_EALLOC);
    byte_index = 0;
    while (byte_index < 16)
    {
        if (original_buffer[byte_index] != 'X')
        {
            cma_free(original_buffer);
            return (0);
        }
        byte_index++;
    }
    cma_free(original_buffer);
    return (1);
}

FT_TEST(test_cma_malloc_limit_sets_errno, "cma_malloc reports allocation failures")
{
    void *allocation_pointer;
    int allocation_errno;

    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(16);
    allocation_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT(allocation_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_errno, FT_EALLOC);
    FT_ASSERT_EQ(ft_errno, FT_EALLOC);
    return (1);
}

FT_TEST(test_cma_malloc_success_sets_errno, "cma_malloc reports success on allocation")
{
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    allocation_pointer = cma_malloc(32);
    if (!allocation_pointer)
        return (0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_realloc_success_sets_errno, "cma_realloc reports success on growth")
{
    char *original_buffer;
    void *reallocation_pointer;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char *>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'Z', 16);
    ft_errno = FT_EALLOC;
    reallocation_pointer = cma_realloc(original_buffer, 64);
    if (!reallocation_pointer)
    {
        cma_free(original_buffer);
        return (0);
    }
    byte_index = 0;
    while (byte_index < 16)
    {
        if (static_cast<char *>(reallocation_pointer)[byte_index] != 'Z')
        {
            cma_free(reallocation_pointer);
            return (0);
        }
        byte_index++;
    }
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(reallocation_pointer);
    return (1);
}

FT_TEST(test_cma_memdup_copies_buffer, "cma_memdup duplicates raw bytes")
{
    unsigned char source[5];
    unsigned char *duplicate;

    source[0] = 0x10;
    source[1] = 0x20;
    source[2] = 0x30;
    source[3] = 0x40;
    source[4] = 0x50;
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(0, ft_memcmp(source, duplicate, sizeof(source)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_zero_size_returns_valid_block, "cma_memdup returns a block when size is zero")
{
    unsigned char source;
    void *duplicate;

    source = 0xAB;
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = cma_memdup(&source, 0);
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_null_source_sets_errno, "cma_memdup rejects null source pointers when size is non-zero")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cma_memdup(ft_nullptr, 4));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_memdup_allocation_failure_sets_errno, "cma_memdup propagates allocation failures")
{
    void *duplicate;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    duplicate = cma_memdup("buffer", 16);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}
