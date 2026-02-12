#include "../test_internal.hpp"
#include "../../Template/deque.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_deque_move_constructor_recreates_mutex,
        "ft_deque move constructor rebuilds mutex and preserves order")
{
    ft_deque<int> source_deque;

    source_deque.push_back(3);
    source_deque.push_back(7);
    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    ft_deque<int> moved_deque(ft_move(source_deque));

    FT_ASSERT(moved_deque.is_thread_safe());
    FT_ASSERT_EQ(false, source_deque.is_thread_safe());
    FT_ASSERT_EQ(2u, moved_deque.size());
    FT_ASSERT_EQ(3, moved_deque.pop_front());
    FT_ASSERT_EQ(7, moved_deque.pop_front());
    FT_ASSERT(moved_deque.empty());
    FT_ASSERT_EQ(0, moved_deque.get_error());
    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_deque_move_assignment_resets_source_mutex,
        "ft_deque move assignment recreates mutex and moves nodes")
{
    ft_deque<int> destination_deque;
    ft_deque<int> source_deque;

    destination_deque.push_back(1);
    FT_ASSERT_EQ(0, destination_deque.enable_thread_safety());
    FT_ASSERT(destination_deque.is_thread_safe());

    source_deque.push_back(9);
    source_deque.push_back(11);
    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    destination_deque = ft_move(source_deque);

    FT_ASSERT(destination_deque.is_thread_safe());
    FT_ASSERT_EQ(false, source_deque.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_deque.size());
    FT_ASSERT_EQ(9, destination_deque.pop_front());
    FT_ASSERT_EQ(11, destination_deque.pop_front());
    FT_ASSERT(destination_deque.empty());
    FT_ASSERT_EQ(0, destination_deque.get_error());
    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_deque_move_supports_reuse_of_source,
        "ft_deque move assignment leaves source reusable with fresh mutex")
{
    ft_deque<int> source_deque;
    ft_deque<int> destination_deque;

    source_deque.push_back(5);
    source_deque.push_back(6);
    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());

    destination_deque = ft_move(source_deque);

    FT_ASSERT(destination_deque.is_thread_safe());
    FT_ASSERT_EQ(false, source_deque.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_deque.size());
    FT_ASSERT_EQ(5, destination_deque.pop_front());
    FT_ASSERT_EQ(6, destination_deque.pop_front());

    FT_ASSERT_EQ(0, source_deque.enable_thread_safety());
    FT_ASSERT(source_deque.is_thread_safe());
    source_deque.push_front(8);
    source_deque.push_back(9);
    FT_ASSERT_EQ(2u, source_deque.size());
    FT_ASSERT_EQ(8, source_deque.pop_front());
    FT_ASSERT_EQ(9, source_deque.pop_front());
    FT_ASSERT(source_deque.empty());
    FT_ASSERT_EQ(0, source_deque.get_error());
    FT_ASSERT_EQ(0, destination_deque.get_error());
    return (1);
}
