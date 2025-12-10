#include "../../Template/queue.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_queue_move_constructor_rebuilds_mutex,
        "ft_queue move constructor rebuilds thread-safety while preserving ordering")
{
    ft_queue<int> source_queue;
    int dequeued_value;

    source_queue.enqueue(5);
    source_queue.enqueue(9);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    ft_queue<int> moved_queue(ft_move(source_queue));

    FT_ASSERT(moved_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_queue.size());

    dequeued_value = moved_queue.dequeue();
    FT_ASSERT_EQ(5, dequeued_value);
    dequeued_value = moved_queue.dequeue();
    FT_ASSERT_EQ(9, dequeued_value);
    FT_ASSERT(moved_queue.empty());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved_queue.get_error());
    return (1);
}

FT_TEST(test_ft_queue_move_assignment_rebuilds_mutex,
        "ft_queue move assignment rebuilds thread-safety while replacing items")
{
    ft_queue<int> destination_queue;
    ft_queue<int> source_queue;
    int dequeued_value;

    destination_queue.enqueue(1);
    destination_queue.enqueue(2);
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());
    FT_ASSERT(destination_queue.is_thread_safe());

    source_queue.enqueue(13);
    source_queue.enqueue(21);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    destination_queue = ft_move(source_queue);

    FT_ASSERT(destination_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_queue.size());

    dequeued_value = destination_queue.dequeue();
    FT_ASSERT_EQ(13, dequeued_value);
    dequeued_value = destination_queue.dequeue();
    FT_ASSERT_EQ(21, dequeued_value);
    FT_ASSERT(destination_queue.empty());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination_queue.get_error());
    return (1);
}

FT_TEST(test_ft_queue_move_preserves_disabled_thread_safety,
        "ft_queue move constructor keeps thread-safety disabled when source was unlocked")
{
    ft_queue<int> source_queue;
    int dequeued_value;

    source_queue.enqueue(4);
    source_queue.enqueue(6);
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());

    ft_queue<int> moved_queue(ft_move(source_queue));

    FT_ASSERT_EQ(false, moved_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_queue.size());
    dequeued_value = moved_queue.dequeue();
    FT_ASSERT_EQ(4, dequeued_value);
    dequeued_value = moved_queue.dequeue();
    FT_ASSERT_EQ(6, dequeued_value);
    return (1);
}

FT_TEST(test_ft_queue_move_allows_reusing_source_after_transfer,
        "ft_queue moved-from object can be reset and enqueued after move")
{
    ft_queue<int> source_queue;
    ft_queue<int> moved_queue;

    source_queue.enqueue(18);
    source_queue.enqueue(22);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    moved_queue = ft_move(source_queue);

    FT_ASSERT(moved_queue.is_thread_safe());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(18, moved_queue.dequeue());
    FT_ASSERT_EQ(22, moved_queue.dequeue());
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());
    source_queue.enqueue(30);
    FT_ASSERT_EQ(30, source_queue.dequeue());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source_queue.get_error());
    return (1);
}
