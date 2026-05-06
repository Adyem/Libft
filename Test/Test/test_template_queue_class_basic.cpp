#include "../test_internal.hpp"
#include "../../Modules/Template/queue.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_queue_class_basic)
{
    ft_queue<int> q;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, q.initialize());
    q.enqueue(1);
    q.enqueue(2);
    FT_ASSERT_EQ(1, q.front());
    FT_ASSERT_EQ(2, q.size());
    int first = q.dequeue();
    FT_ASSERT_EQ(1, first);
    int second = q.dequeue();
    FT_ASSERT_EQ(2, second);
    q.dequeue();
    FT_ASSERT(q.empty());
    return (1);
}
