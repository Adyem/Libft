#include "../test_internal.hpp"
#include "../../Template/deque.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_deque_move_constructor_recreates_mutex,
        "ft_deque manual transfer rebuilds mutex and preserves order")
{
    ft_deque<int> source_deque;
    ft_deque<int> target_deque;

    source_deque.push_back(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    source_deque.push_back(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    while (!source_deque.empty())
    {
        target_deque.push_back(source_deque.pop_front());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    }

    FT_ASSERT(target_deque.empty() == false);
    FT_ASSERT_EQ(2UL, target_deque.size());
    FT_ASSERT_EQ(3, target_deque.pop_front());
    FT_ASSERT_EQ(7, target_deque.pop_front());
    FT_ASSERT(target_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_deque_move_assignment_resets_source_mutex,
        "ft_deque manual assignment recreates mutex and moves nodes")
{
    ft_deque<int> destination_deque;
    ft_deque<int> source_deque;

    destination_deque.push_back(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_deque.enable_thread_safety());
    FT_ASSERT(destination_deque.is_thread_safe());

    source_deque.push_back(9);
    source_deque.push_back(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    while (!source_deque.empty())
    {
        destination_deque.push_back(source_deque.pop_front());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    }

    FT_ASSERT(destination_deque.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_deque.size());
    FT_ASSERT_EQ(9, destination_deque.pop_front());
    FT_ASSERT_EQ(11, destination_deque.pop_front());
    FT_ASSERT(destination_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_deque_move_supports_reuse_of_source,
        "ft_deque manual reuse leaves source ready for reuse")
{
    ft_deque<int> source_deque;
    ft_deque<int> destination_deque;

    source_deque.push_back(5);
    source_deque.push_back(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    while (!source_deque.empty())
    {
        destination_deque.push_back(source_deque.pop_front());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    }

    FT_ASSERT(source_deque.is_thread_safe() == false);
    FT_ASSERT_EQ(2UL, destination_deque.size());
    FT_ASSERT_EQ(5, destination_deque.pop_front());
    FT_ASSERT_EQ(6, destination_deque.pop_front());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    source_deque.push_front(8);
    source_deque.push_back(9);
    FT_ASSERT_EQ(2UL, source_deque.size());
    FT_ASSERT_EQ(8, source_deque.pop_front());
    FT_ASSERT_EQ(9, source_deque.pop_back());
    FT_ASSERT(source_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    return (1);
}
