#include "../test_internal.hpp"
#include "../../Template/queue.hpp"
#include "../../System_utils/test_runner.hpp"
#include <vector>

#ifndef LIBFT_TEST_BUILD
#endif

using queue_type = ft_queue<int>;

static void enqueue_values(queue_type &queue_instance, const std::vector<int> &values)
{
    for (int const value : values)
        queue_instance.enqueue(value);
}

FT_TEST(test_ft_queue_move_constructor_rebuilds_mutex,
        "ft_queue move constructor rebuilds thread-safety while preserving ordering")
{
    queue_type source_queue;
    queue_type destination_queue;
    std::vector<int> values = {5, 9};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    enqueue_values(source_queue, values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.initialize());
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());
    while (!source_queue.empty())
        destination_queue.enqueue(source_queue.dequeue());

    FT_ASSERT_EQ(values.size(), destination_queue.size());
    for (int expected : values)
        FT_ASSERT_EQ(expected, destination_queue.dequeue());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    return (1);
}

FT_TEST(test_ft_queue_move_assignment_rebuilds_mutex,
        "ft_queue move assignment reinitializes mutex and replaces items")
{
    queue_type destination_queue;
    queue_type source_queue;
    std::vector<int> source_values = {13, 21};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.initialize());
    enqueue_values(destination_queue, {1, 2});
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    enqueue_values(source_queue, source_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.initialize());
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());
    while (!source_queue.empty())
        destination_queue.enqueue(source_queue.dequeue());

    FT_ASSERT_EQ(source_values.size(), destination_queue.size());
    for (int expected : source_values)
        FT_ASSERT_EQ(expected, destination_queue.dequeue());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    return (1);
}

FT_TEST(test_ft_queue_move_preserves_disabled_thread_safety,
        "ft_queue move constructor keeps thread-safety disabled when source was unlocked")
{
    queue_type source_queue;
    queue_type destination_queue;
    std::vector<int> values = {4, 6};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    enqueue_values(source_queue, values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.initialize());
    enqueue_values(destination_queue, values);
    FT_ASSERT_EQ(false, destination_queue.is_thread_safe());

    FT_ASSERT_EQ(values.size(), destination_queue.size());
    for (int expected : values)
        FT_ASSERT_EQ(expected, destination_queue.dequeue());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    return (1);
}

FT_TEST(test_ft_queue_move_allows_reusing_source_after_transfer,
        "ft_queue moved-from object can be reset and enqueued after move")
{
    queue_type source_queue;
    std::vector<int> values = {18, 22};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    enqueue_values(source_queue, values);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());

    for (int expected : values)
        FT_ASSERT_EQ(expected, source_queue.dequeue());

    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());
    source_queue.enqueue(30);
    FT_ASSERT_EQ(30, source_queue.dequeue());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.destroy());
    return (1);
}
