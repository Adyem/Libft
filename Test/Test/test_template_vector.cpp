#include "../test_internal.hpp"
#include "../../Template/vector.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_vector_ft_string_growth, "ft_vector preserves ft_string contents across reallocations")
{
    ft_size_t allocation_count_before = 0;
    ft_size_t free_count_before = 0;
    cma_get_stats(&allocation_count_before, &free_count_before);
    {
        ft_vector<ft_string> string_vector;
        const char* sample_values[7] = {
            "alpha",
            "bravo",
            "charlie",
            "delta",
            "echo",
            "foxtrot",
            "golf"
        };
        size_t push_index = 0;
        while (push_index < 7)
        {
            string_vector.push_back(ft_string(sample_values[push_index]));
            ++push_index;
        }
        FT_ASSERT_EQ(7, string_vector.size());
        FT_ASSERT(string_vector.capacity() >= 7);
        size_t verify_index = 0;
        while (verify_index < 7)
        {
            FT_ASSERT(string_vector[verify_index] == sample_values[verify_index]);
            ++verify_index;
        }
        FT_ASSERT_EQ(FT_ERR_SUCCESS, string_vector.get_error());
    }
    ft_size_t allocation_count_after = 0;
    ft_size_t free_count_after = 0;
    cma_get_stats(&allocation_count_after, &free_count_after);
    FT_ASSERT(allocation_count_after >= allocation_count_before);
    FT_ASSERT(free_count_after >= free_count_before);
    ft_size_t allocation_delta = allocation_count_after - allocation_count_before;
    ft_size_t free_delta = free_count_after - free_count_before;
    FT_ASSERT_EQ(allocation_delta, free_delta);
    return (1);
}

FT_TEST(test_ft_vector_small_buffer_inline_capacity, "ft_vector uses inline storage before allocating from CMA")
{
    ft_size_t allocation_count_before = 0;
    ft_size_t free_count_before = 0;
    cma_get_stats(&allocation_count_before, &free_count_before);
    {
        ft_vector<int> inline_vector;
        size_t inline_capacity = inline_vector.capacity();
        FT_ASSERT(inline_capacity > 0);
        size_t push_index = 0;
        while (push_index < inline_capacity)
        {
            inline_vector.push_back(static_cast<int>(push_index));
            ++push_index;
        }
        ft_size_t allocation_count_mid = 0;
        ft_size_t free_count_mid = 0;
        cma_get_stats(&allocation_count_mid, &free_count_mid);
        FT_ASSERT_EQ(allocation_count_before, allocation_count_mid);
        FT_ASSERT_EQ(free_count_before, free_count_mid);
        size_t verify_index = 0;
        while (verify_index < inline_vector.size())
        {
            FT_ASSERT_EQ(static_cast<int>(verify_index), inline_vector[verify_index]);
            ++verify_index;
        }
        FT_ASSERT_EQ(inline_vector.capacity(), inline_capacity);
    }
    ft_size_t allocation_count_after = 0;
    ft_size_t free_count_after = 0;
    cma_get_stats(&allocation_count_after, &free_count_after);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    FT_ASSERT_EQ(free_count_before, free_count_after);
    return (1);
}
