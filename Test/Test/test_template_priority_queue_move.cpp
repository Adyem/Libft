#include "../test_internal.hpp"
#include "../../Modules/Template/priority_queue.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_priority_queue_move_constructor_rebuilds_mutex)
{
    ft_priority_queue<int> source_queue;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    source_queue.push(5);
    source_queue.push(12);
    source_queue.push(7);
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT(source_queue.is_thread_safe());

    ft_priority_queue<int> new_queue;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_queue.move(source_queue));

    FT_ASSERT(new_queue.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.move(source_queue));
    FT_ASSERT(destination_queue.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_queue.size());
    FT_ASSERT_EQ(20, destination_queue.top());
    FT_ASSERT_EQ(20, destination_queue.pop());
    FT_ASSERT_EQ(15, destination_queue.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.initialize());
    FT_ASSERT_EQ(false, source_queue.is_thread_safe());
    FT_ASSERT_EQ(0, source_queue.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_queue.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_queue.destroy());
    return (1);
}
