#include "../test_internal.hpp"
#include "../../Template/deque.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int deque_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_deque_get_error_returned = FT_FALSE;
static int32_t g_deque_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_deque_get_error_str_returned = FT_FALSE;
static const char *g_deque_get_error_str_result = ft_nullptr;

static void deque_get_error_uninitialised_operation(void)
{
    ft_deque<int> deque_value;

    g_deque_get_error_result = deque_value.get_error();
    g_deque_get_error_returned = FT_TRUE;
    return ;
}

static void deque_get_error_str_uninitialised_operation(void)
{
    ft_deque<int> deque_value;

    g_deque_get_error_str_result = deque_value.get_error_str();
    g_deque_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_deque_move_constructor_recreates_mutex)
{
    ft_deque<int> source_deque;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    source_deque.push_back(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    source_deque.push_back(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    ft_deque<int> target_deque(ft_move(source_deque));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.get_error());
    FT_ASSERT(target_deque.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    FT_ASSERT_EQ(false, source_deque.is_thread_safe());
    FT_ASSERT_EQ(2UL, target_deque.size());
    FT_ASSERT_EQ(3, target_deque.pop_front());
    FT_ASSERT_EQ(7, target_deque.pop_front());
    FT_ASSERT(target_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.destroy());
    return (1);
}

FT_TEST(test_ft_deque_move_assignment_resets_source_mutex)
{
    ft_deque<int> destination_deque;
    ft_deque<int> source_deque;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    destination_deque.push_back(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.enable_thread_safety());
    FT_ASSERT(destination_deque.is_thread_safe());

    source_deque.push_back(9);
    source_deque.push_back(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.move(source_deque));
    FT_ASSERT(destination_deque.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    FT_ASSERT_EQ(false, source_deque.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_deque.size());
    FT_ASSERT_EQ(9, destination_deque.pop_front());
    FT_ASSERT_EQ(11, destination_deque.pop_front());
    FT_ASSERT(destination_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.destroy());
    return (1);
}

FT_TEST(test_ft_deque_move_supports_reuse_of_source)
{
    ft_deque<int> source_deque;
    ft_deque<int> destination_deque;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.initialize());
    source_deque.push_back(5);
    source_deque.push_back(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.move(source_deque));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.get_error());
    FT_ASSERT(destination_deque.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_deque.size());
    FT_ASSERT_EQ(5, destination_deque.pop_front());
    FT_ASSERT_EQ(6, destination_deque.pop_front());
    FT_ASSERT(destination_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    source_deque.push_front(8);
    source_deque.push_back(9);
    FT_ASSERT_EQ(2UL, source_deque.size());
    FT_ASSERT_EQ(8, source_deque.pop_front());
    FT_ASSERT_EQ(9, source_deque.pop_back());
    FT_ASSERT(source_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.destroy());
    return (1);
}

FT_TEST(test_ft_deque_error_queries_follow_lifecycle_contract)
{
    ft_deque<int> deque_value;

    g_deque_get_error_returned = FT_FALSE;
    g_deque_get_error_result = FT_ERR_SUCCESS;
    g_deque_get_error_str_returned = FT_FALSE;
    g_deque_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, deque_expect_sigabrt(
        deque_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_deque_get_error_returned);
    FT_ASSERT_EQ(1, deque_expect_sigabrt(
        deque_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_deque_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(deque_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
