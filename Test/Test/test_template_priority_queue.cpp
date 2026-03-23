#include "../test_internal.hpp"
#include "../../Template/priority_queue.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <functional>
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_priority_queue_push_pop_order)
{
    ft_priority_queue<int> queue_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    queue_instance.push(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    queue_instance.push(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    queue_instance.push(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    FT_ASSERT_EQ(false, queue_instance.empty());
    FT_ASSERT_EQ(3UL, queue_instance.size());
    FT_ASSERT_EQ(9, queue_instance.top());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    int first_value = queue_instance.pop();
    FT_ASSERT_EQ(9, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(2UL, queue_instance.size());

    int second_value = queue_instance.pop();
    FT_ASSERT_EQ(5, second_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    int third_value = queue_instance.pop();
    FT_ASSERT_EQ(1, third_value);
    FT_ASSERT(queue_instance.empty());
    FT_ASSERT_EQ(0UL, queue_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}

FT_TEST(test_ft_priority_queue_custom_comparator)
{
    ft_priority_queue<int, std::greater<int> > queue_instance(0, std::greater<int>());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    queue_instance.push(4);
    queue_instance.push(2);
    queue_instance.push(7);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(3UL, queue_instance.size());
    FT_ASSERT_EQ(2, queue_instance.top());

    int first_value = queue_instance.pop();
    FT_ASSERT_EQ(2, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(4, queue_instance.top());

    queue_instance.pop();
    queue_instance.pop();
    FT_ASSERT(queue_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}

FT_TEST(test_ft_priority_queue_error_handling_and_moves)
{
    ft_priority_queue<int> queue_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    int empty_value = queue_instance.pop();
    FT_ASSERT_EQ(0, empty_value);
    FT_ASSERT_EQ(FT_ERR_PRIORITY_QUEUE_EMPTY, queue_instance.get_error());

    int top_empty = queue_instance.top();
    FT_ASSERT_EQ(0, top_empty);
    FT_ASSERT_EQ(FT_ERR_PRIORITY_QUEUE_EMPTY, queue_instance.get_error());

    queue_instance.push(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    queue_instance.push(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    ft_priority_queue<int> recreated_queue;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.initialize());
    recreated_queue.push(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.get_error());
    recreated_queue.push(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.get_error());

    FT_ASSERT_EQ(false, recreated_queue.empty());
    FT_ASSERT_EQ(2UL, recreated_queue.size());
    FT_ASSERT_EQ(11, recreated_queue.top());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.get_error());

    int popped_value = recreated_queue.pop();
    FT_ASSERT_EQ(11, popped_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.get_error());

    recreated_queue.clear();
    FT_ASSERT(recreated_queue.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recreated_queue.destroy());
    return (1);
}
