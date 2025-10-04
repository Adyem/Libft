#include "../../Template/promise.hpp"
#include "../../Template/atomic.hpp"
#include "../../PThread/pthread.hpp"
#include "../../PThread/thread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_promise_thread_safe_set_and_get, "ft_promise synchronizes concurrent setters and getters")
{
    ft_promise<int> promise_instance;
    ft_atomic<int> successful_gets(0);

    auto setter = [&promise_instance]()
    {
        int value;
        value = 0;
        while (value < 100)
        {
            promise_instance.set_value(value);
            value = value + 1;
        }
        return ;
    };
    auto getter = [&promise_instance, &successful_gets]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 100)
        {
            if (!promise_instance.is_ready())
            {
                pt_thread_yield();
                continue ;
            }
            promise_instance.get();
            if (promise_instance.get_error() == ER_SUCCESS)
            {
                successful_gets.fetch_add(1);
                iteration = iteration + 1;
            }
        }
        return ;
    };

    ft_thread setter_thread(setter);
    ft_thread getter_thread(getter);

    setter_thread.join();
    getter_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, setter_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, getter_thread.get_error());
    FT_ASSERT_EQ(100, successful_gets.load());
    FT_ASSERT_EQ(ER_SUCCESS, promise_instance.get_error());
    return (1);
}

FT_TEST(test_template_promise_thread_safe_ordered_writes, "ft_promise handles ordered updates from multiple threads")
{
    ft_promise<int> promise_instance;
    ft_atomic<int> stage(0);

    auto first_setter = [&promise_instance, &stage]()
    {
        while (stage.load() != 0)
        {
            pt_thread_yield();
        }
        promise_instance.set_value(17);
        stage.store(1);
        return ;
    };
    auto second_setter = [&promise_instance, &stage]()
    {
        while (stage.load() < 1)
        {
            pt_thread_yield();
        }
        promise_instance.set_value(23);
        stage.store(2);
        return ;
    };

    ft_thread first_thread(first_setter);
    ft_thread second_thread(second_setter);

    first_thread.join();
    second_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_thread.get_error());
    FT_ASSERT_EQ(2, stage.load());
    FT_ASSERT_EQ(23, promise_instance.get());
    FT_ASSERT_EQ(ER_SUCCESS, promise_instance.get_error());
    return (1);
}

FT_TEST(test_template_promise_void_thread_safety, "ft_promise<void> synchronizes readiness checks")
{
    ft_promise<void> promise_instance;
    ft_atomic<int> ready_checks(0);

    auto setter = [&promise_instance]()
    {
        pt_thread_sleep(30);
        promise_instance.set_value();
        return ;
    };
    auto readiness_checker = [&promise_instance, &ready_checks]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 60)
        {
            if (promise_instance.is_ready())
            {
                ready_checks.fetch_add(1);
            }
            else
            {
                pt_thread_yield();
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto getter = [&promise_instance]()
    {
        promise_instance.get();
        return ;
    };

    ft_thread setter_thread(setter);
    ft_thread readiness_thread(readiness_checker);
    ft_thread getter_thread(getter);

    setter_thread.join();
    readiness_thread.join();
    getter_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, setter_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, readiness_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, getter_thread.get_error());
    FT_ASSERT(ready_checks.load() > 0);
    FT_ASSERT_EQ(ER_SUCCESS, promise_instance.get_error());
    return (1);
}
