#include "../test_internal.hpp"
#include "../../Template/priority_queue.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_priority_queue_move_constructor_rebuilds_mutex)
{
    ft_priority_queue<int> source_queue;
    ft_priority_queue<int> new_queue;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    source_queue.push(5);
    source_queue.push(12);
    source_queue.push(7);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_queue.initialize());
    while (!source_queue.empty())
    {
        new_queue.push(source_queue.pop());
        FT_ASSERT_EQ(FT_ERR_SUCCESS,
                new_queue.get_error());
    }
    FT_ASSERT(source_queue.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_queue.get_error());
    FT_ASSERT_EQ(3UL, new_queue.size());
    FT_ASSERT_EQ(12, new_queue.top());
    FT_ASSERT_EQ(12, new_queue.pop());
    FT_ASSERT_EQ(7, new_queue.top());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_queue.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.destroy());
    return (1);
}

FT_TEST(test_ft_priority_queue_move_assignment_allows_reenable)
{
    ft_priority_queue<int> destination_queue;
    ft_priority_queue<int> source_queue;
    ft_priority_queue<int> temp_queue;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.initialize());
    destination_queue.push(1);
    FT_ASSERT_EQ(0, destination_queue.enable_thread_safety());
    FT_ASSERT(destination_queue.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    source_queue.push(20);
    source_queue.push(15);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    FT_ASSERT(destination_queue.is_thread_safe());
    while (!destination_queue.empty())
        destination_queue.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, temp_queue.initialize());
    while (!source_queue.empty())
    {
        temp_queue.push(source_queue.pop());
        FT_ASSERT_EQ(FT_ERR_SUCCESS,
                temp_queue.get_error());
    }
    FT_ASSERT(destination_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, temp_queue.size());
    FT_ASSERT_EQ(20, temp_queue.top());
    FT_ASSERT_EQ(20, temp_queue.pop());
    FT_ASSERT_EQ(15, temp_queue.pop());
    FT_ASSERT(source_queue.empty());
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, temp_queue.destroy());
    return (1);
}
