#include "../test_internal.hpp"
#include "../../Template/queue.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_queue_basic)
{
    ft_queue<int> queue;
    int value_a = 1;
    int value_b = 2;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    queue.enqueue(value_a);
    queue.enqueue(value_b);
    FT_ASSERT_EQ(1, queue.front());
    int first = queue.dequeue();
    FT_ASSERT_EQ(1, first);
    int second = queue.dequeue();
    FT_ASSERT_EQ(2, second);
    FT_ASSERT(queue.empty());
    return (1);
}
