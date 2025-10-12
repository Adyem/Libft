#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../cma_test_helpers.hpp"

FT_TEST(test_cma_extended_stats_track_bytes,
        "cma_get_extended_stats reports live and peak usage")
{
    ft_size_t allocation_count_before;
    ft_size_t free_count_before;
    ft_size_t current_bytes_before;
    ft_size_t peak_bytes_before;
    ft_size_t allocation_count_after;
    ft_size_t free_count_after;
    ft_size_t current_bytes_after;
    ft_size_t peak_bytes_after;
    ft_size_t allocation_count_post_free_one;
    ft_size_t free_count_post_free_one;
    ft_size_t current_bytes_post_free_one;
    ft_size_t peak_bytes_post_free_one;
    ft_size_t final_allocation_count;
    ft_size_t final_free_count;
    ft_size_t final_current_bytes;
    ft_size_t final_peak_bytes;
    char *first_block;
    char *second_block;
    ft_size_t first_block_size;
    ft_size_t second_block_size;

    cma_set_alloc_limit(0);
    FT_ASSERT(ensure_cma_cleanup_success());
    cma_get_extended_stats(&allocation_count_before,
            &free_count_before,
            &current_bytes_before,
            &peak_bytes_before);
    first_block = static_cast<char*>(cma_malloc(32));
    if (first_block == ft_nullptr)
        return (0);
    first_block_size = cma_alloc_size(first_block);
    second_block = static_cast<char*>(cma_malloc(48));
    if (second_block == ft_nullptr)
    {
        cma_free(first_block);
        return (0);
    }
    second_block_size = cma_alloc_size(second_block);
    cma_get_extended_stats(&allocation_count_after,
            &free_count_after,
            &current_bytes_after,
            &peak_bytes_after);
    FT_ASSERT_EQ(allocation_count_before + 2, allocation_count_after);
    FT_ASSERT_EQ(free_count_before, free_count_after);
    FT_ASSERT_EQ(current_bytes_before + first_block_size + second_block_size,
            current_bytes_after);
    FT_ASSERT(peak_bytes_after >= peak_bytes_before);
    FT_ASSERT(peak_bytes_after >= current_bytes_after);
    cma_free(first_block);
    cma_get_extended_stats(&allocation_count_post_free_one,
            &free_count_post_free_one,
            &current_bytes_post_free_one,
            &peak_bytes_post_free_one);
    FT_ASSERT_EQ(allocation_count_after, allocation_count_post_free_one);
    FT_ASSERT_EQ(free_count_after + 1, free_count_post_free_one);
    FT_ASSERT_EQ(current_bytes_after - first_block_size,
            current_bytes_post_free_one);
    FT_ASSERT_EQ(peak_bytes_after, peak_bytes_post_free_one);
    cma_free(second_block);
    cma_get_extended_stats(&final_allocation_count,
            &final_free_count,
            &final_current_bytes,
            &final_peak_bytes);
    FT_ASSERT_EQ(allocation_count_post_free_one, final_allocation_count);
    FT_ASSERT_EQ(free_count_post_free_one + 1, final_free_count);
    FT_ASSERT_EQ(current_bytes_before, final_current_bytes);
    FT_ASSERT_EQ(peak_bytes_post_free_one, final_peak_bytes);
    return (1);
}
