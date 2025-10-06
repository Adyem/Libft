#include "../../Template/function.hpp"
#include <atomic>
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/unique_lock.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_function_thread_safe_concurrent_invocation, "ft_function handles concurrent invocations")
{
    std::atomic<int> call_count(0);
    pt_mutex counter_mutex;

    ft_function<void()> function_instance([&call_count, &counter_mutex]()
    {
        ft_unique_lock<pt_mutex> guard(counter_mutex);
        if (guard.get_error() == ER_SUCCESS)
        {
            call_count.fetch_add(1);
        }
        return ;
    });

    auto caller = [&function_instance]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 100)
        {
            function_instance();
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread first_thread(caller);
    ft_thread second_thread(caller);

    first_thread.join();
    second_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_thread.get_error());
    FT_ASSERT_EQ(200, call_count.load());
    FT_ASSERT_EQ(ER_SUCCESS, function_instance.get_error());
    return (1);
}

FT_TEST(test_template_function_thread_safe_assignment_during_calls, "ft_function stays valid while reassigned during calls")
{
    ft_function<int()> function_instance([]()
    {
        return (1);
    });
    std::atomic<int> invocation_sum(0);

    auto invoking_thread = [&function_instance, &invocation_sum]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            int value;
            value = function_instance();
            invocation_sum.fetch_add(value);
            iteration = iteration + 1;
        }
        return ;
    };
    auto assigning_thread = [&function_instance]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            if ((iteration & 1) == 0)
            {
                function_instance = ft_function<int()>([]()
                {
                    return (2);
                });
            }
            else
            {
                function_instance = ft_function<int()>([]()
                {
                    return (3);
                });
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread call_thread(invoking_thread);
    ft_thread assign_thread(assigning_thread);

    call_thread.join();
    assign_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, call_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, assign_thread.get_error());
    int final_result;

    function_instance = ft_function<int()>([]()
    {
        return (4);
    });
    final_result = function_instance();
    FT_ASSERT(invocation_sum.load() >= 120);
    FT_ASSERT(final_result == 4);
    FT_ASSERT_EQ(ER_SUCCESS, function_instance.get_error());
    return (1);
}

FT_TEST(test_template_function_thread_safe_copy_during_use, "ft_function copy operations synchronize with use")
{
    std::atomic<int> total_calls(0);

    ft_function<void()> function_instance([&total_calls]()
    {
        total_calls.fetch_add(1);
        return ;
    });

    auto copier = [&function_instance, &total_calls]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 80)
        {
            ft_function<void()> copy_instance(function_instance);
            copy_instance();
            iteration = iteration + 1;
        }
        return ;
    };
    auto assigner = [&function_instance, &total_calls]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 80)
        {
            ft_function<void()> temp_instance(function_instance);
            function_instance = temp_instance;
            function_instance();
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread copy_thread(copier);
    ft_thread assign_thread(assigner);

    copy_thread.join();
    assign_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, copy_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, assign_thread.get_error());
    FT_ASSERT(total_calls.load() >= 160);
    FT_ASSERT_EQ(ER_SUCCESS, function_instance.get_error());
    return (1);
}
