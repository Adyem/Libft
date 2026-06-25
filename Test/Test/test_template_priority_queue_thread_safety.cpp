#include "../test_internal.hpp"
#include "../../Modules/Template/priority_queue.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

using priority_queue_int = ft_priority_queue<int>;

FT_TEST(test_ft_priority_queue_enable_thread_safety_initializes_mutex)
{
    ft_priority_queue<int> queue_instance;
    ft_bool                lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT(queue_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    queue_instance.push(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    queue_instance.push(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    FT_ASSERT_EQ(10, queue_instance.top());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    int first_value = queue_instance.pop();
    FT_ASSERT_EQ(10, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    queue_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT(queue_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, queue_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_FALSE);
    queue_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}

FT_TEST(test_ft_priority_queue_lock_blocks_until_release)
{
    ft_priority_queue<int> queue_instance;
    ft_bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    main_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, queue_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == FT_TRUE);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&queue_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        ft_bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = FT_FALSE;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (queue_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        queue_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    queue_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}
