#include "../test_internal.hpp"
#include "../../Template/priority_queue.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_priority_queue_move_constructor_rebuilds_mutex,
        "ft_priority_queue move constructor recreates mutex and preserves ordering")
{
    ft_priority_queue<int> source_queue;

    source_queue.push(5);
    source_queue.push(12);
    source_queue.push(7);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    ft_priority_queue<int> moved_queue(ft_move(source_queue));

    FT_ASSERT(moved_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(3UL, moved_queue.size());
    FT_ASSERT_EQ(12, moved_queue.top());
    FT_ASSERT_EQ(12, moved_queue.pop());
    FT_ASSERT_EQ(7, moved_queue.top());
    return (1);
}

FT_TEST(test_ft_priority_queue_move_assignment_allows_reenable,
        "ft_priority_queue move assignment rebuilds mutex and empties source")
{
    ft_priority_queue<int> destination_queue;
    ft_priority_queue<int> source_queue;

    destination_queue.push(1);
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());
    FT_ASSERT(destination_queue.is_thread_safe());

    source_queue.push(20);
    source_queue.push(15);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    destination_queue = ft_move(source_queue);

    FT_ASSERT(destination_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_queue.size());
    FT_ASSERT_EQ(20, destination_queue.top());
    FT_ASSERT_EQ(20, destination_queue.pop());
    FT_ASSERT_EQ(15, destination_queue.pop());
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());
    return (1);
}
