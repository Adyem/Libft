#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/concurrency.hpp"
#include "../../Modules/Threading/cancellation.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_pthread_thread_safe_queue_push_and_pop)
{
    ft_thread_safe_queue<int32_t> queue;
    int32_t value;

    value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    FT_ASSERT_EQ(FT_TRUE, queue.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.push(42));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), queue.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.try_pop(&value));
    FT_ASSERT_EQ(42, value);
    FT_ASSERT_EQ(FT_TRUE, queue.empty());
    FT_ASSERT_EQ(FT_ERR_EMPTY, queue.try_pop(&value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.destroy());
    return (1);
}

FT_TEST(test_pthread_thread_safe_queue_blocking_pop_waits_for_value)
{
    ft_thread_safe_queue<int32_t> queue;
    ft_thread worker;
    int32_t result;
    int32_t value;

    result = FT_ERR_INTERNAL;
    value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    worker = ft_thread([&queue, &result, &value]()
    {
        result = queue.blocking_pop(&value);
        return ;
    });
    pt_thread_sleep(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.push(77));
    worker.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(77, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.destroy());
    return (1);
}

FT_TEST(test_pthread_thread_safe_queue_timeout_and_cancellation)
{
    ft_thread_safe_queue<int32_t> queue;
    ft_cancellation_source source;
    int32_t value;

    value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    FT_ASSERT_EQ(FT_ERR_TIMEOUT, queue.blocking_pop_for(&value, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.request_cancel());
    FT_ASSERT_EQ(FT_ERR_TERMINATED, queue.blocking_pop(&value, source.get_token()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.destroy());
    return (1);
}
