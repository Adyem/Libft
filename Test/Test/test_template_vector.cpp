#include "../test_internal.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int vector_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_vector_get_error_returned = FT_FALSE;
static int32_t g_vector_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_vector_get_error_str_returned = FT_FALSE;
static const char *g_vector_get_error_str_result = ft_nullptr;

static void vector_get_error_uninitialised_operation(void)
{
    ft_vector<int> vector_value;

    g_vector_get_error_result = vector_value.get_error();
    g_vector_get_error_returned = FT_TRUE;
    return ;
}

static void vector_get_error_str_uninitialised_operation(void)
{
    ft_vector<int> vector_value;

    g_vector_get_error_str_result = vector_value.get_error_str();
    g_vector_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_vector_ft_string_growth)
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
        FT_ASSERT_EQ(FT_ERR_SUCCESS, string_vector.initialize());
        while (push_index < 7)
        {
            ft_string stored_value;
            FT_ASSERT_EQ(FT_ERR_SUCCESS, stored_value.initialize(sample_values[push_index]));
            FT_ASSERT_EQ(FT_ERR_SUCCESS, string_vector.push_back(stored_value));
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
        FT_ASSERT_EQ(FT_ERR_SUCCESS, string_vector.destroy());
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

FT_TEST(test_ft_vector_small_buffer_inline_capacity)
{
    ft_size_t allocation_count_before = 0;
    ft_size_t free_count_before = 0;
    cma_get_stats(&allocation_count_before, &free_count_before);
    {
        ft_vector<int> inline_vector;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, inline_vector.initialize());
        size_t inline_capacity = inline_vector.capacity();
        FT_ASSERT(inline_capacity > 0);
        size_t push_index = 0;
        while (push_index < inline_capacity)
        {
            FT_ASSERT_EQ(FT_ERR_SUCCESS, inline_vector.push_back(static_cast<int>(push_index)));
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
        FT_ASSERT_EQ(FT_ERR_SUCCESS, inline_vector.destroy());
    }
    ft_size_t allocation_count_after = 0;
    ft_size_t free_count_after = 0;
    cma_get_stats(&allocation_count_after, &free_count_after);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    FT_ASSERT_EQ(free_count_before, free_count_after);
    return (1);
}

FT_TEST(test_ft_vector_move_method_transfers_elements_and_thread_safety)
{
    ft_vector<int> source_vector;
    ft_vector<int> destination_vector;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_vector.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_vector.push_back(10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_vector.push_back(20));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_vector.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_vector.move(source_vector));
    FT_ASSERT_EQ(2U, destination_vector.size());
    FT_ASSERT_EQ(10, destination_vector[0]);
    FT_ASSERT_EQ(20, destination_vector[1]);
    FT_ASSERT_EQ(FT_TRUE, destination_vector.is_thread_safe());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_vector.is_initialised());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_vector.destroy());
    return (1);
}

FT_TEST(test_ft_vector_error_queries_follow_lifecycle_contract)
{
    ft_vector<int> vector_value;

    g_vector_get_error_returned = FT_FALSE;
    g_vector_get_error_result = FT_ERR_SUCCESS;
    g_vector_get_error_str_returned = FT_FALSE;
    g_vector_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, vector_expect_sigabrt(
        vector_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_vector_get_error_returned);
    FT_ASSERT_EQ(1, vector_expect_sigabrt(
        vector_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_vector_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(vector_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
