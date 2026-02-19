#include "../test_internal.hpp"
#include "../../Template/promise.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/condition.hpp"
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_promise_set_get, "ft_promise set and get")
{
    ft_promise<int> promise_instance;

    promise_instance.set_value(42);
    FT_ASSERT(promise_instance.is_ready());
    FT_ASSERT_EQ(42, promise_instance.get());
    return (1);
}

FT_TEST(test_ft_promise_not_ready, "ft_promise not ready")
{
    ft_promise<int> promise_instance;

    promise_instance.get();
    return (1);
}

FT_TEST(test_pt_async_basic, "pt_async basic")
{
    ft_promise<int> promise_instance;

    if (pt_async(promise_instance, []() { return (7); }) != 0)
        return (0);
    while (!promise_instance.is_ready())
        usleep(1000);
    FT_ASSERT_EQ(7, promise_instance.get());
    return (1);
}

FT_TEST(test_pt_cond_wait_signal, "pt_cond wait signal")
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ready;

    pthread_mutex_init(&mutex, ft_nullptr);
    pt_cond_init(&condition, ft_nullptr);
    ready = 0;
    struct condition_data
    {
        pthread_mutex_t *mutex;
        pthread_cond_t *condition;
        int *ready;
    };
    condition_data data = { &mutex, &condition, &ready };
    auto start_routine = [](void *argument) -> void*
    {
        condition_data *condition_info;

        condition_info = static_cast<condition_data*>(argument);
        pthread_mutex_lock(condition_info->mutex);
        while (*(condition_info->ready) == 0)
            pt_cond_wait(condition_info->condition, condition_info->mutex);
        pthread_mutex_unlock(condition_info->mutex);
        return (ft_nullptr);
    };
    pthread_t thread;
    if (pt_thread_create(&thread, ft_nullptr, start_routine, &data) != 0)
    {
        pt_cond_destroy(&condition);
        pthread_mutex_destroy(&mutex);
        return (0);
    }
    pt_thread_sleep(100);
    pthread_mutex_lock(&mutex);
    ready = 1;
    pt_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    int join_result;

    join_result = pt_thread_join(thread, ft_nullptr);
    pt_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    FT_ASSERT_EQ(0, join_result);
    FT_ASSERT_EQ(1, ready);
    return (1);
}
