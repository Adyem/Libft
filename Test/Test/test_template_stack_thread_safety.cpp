#include "../test_internal.hpp"
#include "../../Template/stack.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_stack_enable_thread_safety_initializes_mutex,
        "ft_stack installs optional mutex guards when requested")
{
    ft_stack<int> stack_instance;

    FT_ASSERT_EQ(0, stack_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    FT_ASSERT(stack_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());

    stack_instance.push(42);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    FT_ASSERT_EQ(false, stack_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());

    int value = stack_instance.pop();
    FT_ASSERT_EQ(42, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    FT_ASSERT(stack_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());

    stack_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    FT_ASSERT(stack_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());

    bool lock_acquired = false;
    FT_ASSERT_EQ(0, stack_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    stack_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_stack_lock_blocks_until_release,
        "ft_stack lock waits for mutex holders to release")
{
    ft_stack<int> stack_instance;
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(0, stack_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, stack_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&stack_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (stack_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        stack_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stack_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    stack_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_stack<int>::last_operation_error());
    return (1);
}
