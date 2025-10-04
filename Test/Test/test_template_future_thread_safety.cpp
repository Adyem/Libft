#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/atomic.hpp"
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_future_thread_safe_wait_and_get, "ft_future coordinates wait and get across threads")
{
    ft_promise<int> promise_instance;
    ft_future<int> future_instance(promise_instance);
    ft_atomic<int> success_counter(0);

    auto wait_routine = [&future_instance, &success_counter]()
    {
        future_instance.wait();
        if (future_instance.get_error() == ER_SUCCESS)
        {
            success_counter.fetch_add(1);
        }
        return ;
    };
    auto get_routine = [&future_instance, &success_counter]()
    {
        int value;

        value = future_instance.get();
        if (value == 42 && future_instance.get_error() == ER_SUCCESS)
        {
            success_counter.fetch_add(1);
        }
        return ;
    };

    ft_thread wait_thread(wait_routine);
    ft_thread get_thread(get_routine);

    pt_thread_sleep(50);
    promise_instance.set_value(42);

    wait_thread.join();
    get_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, wait_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, get_thread.get_error());
    FT_ASSERT_EQ(2, success_counter.load());
    FT_ASSERT_EQ(ER_SUCCESS, future_instance.get_error());
    return (1);
}

FT_TEST(test_template_future_thread_safe_multiple_gets, "ft_future allows simultaneous getters")
{
    ft_promise<int> promise_instance;
    ft_future<int> future_instance(promise_instance);
    ft_atomic<int> sum_values(0);

    auto getter = [&future_instance, &sum_values]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 3)
        {
            int value;

            value = future_instance.get();
            if (future_instance.get_error() == ER_SUCCESS)
            {
                sum_values.fetch_add(value);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread first_getter(getter);
    ft_thread second_getter(getter);

    pt_thread_sleep(30);
    promise_instance.set_value(7);

    first_getter.join();
    second_getter.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_getter.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_getter.get_error());
    FT_ASSERT_EQ(6 * 7, sum_values.load());
    FT_ASSERT_EQ(ER_SUCCESS, future_instance.get_error());
    return (1);
}

FT_TEST(test_template_future_void_thread_safety, "ft_future<void> synchronizes waiters and setters")
{
    ft_promise<void> promise_instance;
    ft_future<void> future_instance(promise_instance);
    ft_atomic<int> completion_counter(0);

    auto waiter = [&future_instance, &completion_counter]()
    {
        future_instance.wait();
        if (future_instance.get_error() == ER_SUCCESS)
        {
            completion_counter.fetch_add(1);
        }
        return ;
    };
    auto getter = [&future_instance, &completion_counter]()
    {
        future_instance.get();
        if (future_instance.get_error() == ER_SUCCESS)
        {
            completion_counter.fetch_add(1);
        }
        return ;
    };

    ft_thread wait_thread(waiter);
    ft_thread get_thread(getter);

    pt_thread_sleep(40);
    promise_instance.set_value();

    wait_thread.join();
    get_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, wait_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, get_thread.get_error());
    FT_ASSERT_EQ(2, completion_counter.load());
    FT_ASSERT_EQ(ER_SUCCESS, future_instance.get_error());
    return (1);
}
