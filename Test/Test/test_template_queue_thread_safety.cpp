#include "../test_internal.hpp"
#include "../../Template/queue.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#ifndef LIBFT_TEST_BUILD
#endif

using queue_type = ft_queue<int>;

FT_TEST(test_ft_queue_enable_thread_safety_initializes_mutex,
        "ft_queue installs optional mutex guards when requested")
{
    queue_type queue_instance;
    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT(queue_instance.is_thread_safe());

    queue_instance.enqueue(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT_EQ(false, queue_instance.empty());

    int value = queue_instance.dequeue();
    FT_ASSERT_EQ(7, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT(queue_instance.empty());

    queue_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    FT_ASSERT(queue_instance.is_thread_safe() == false);

    bool lock_acquired = false;
    FT_ASSERT_EQ(0, queue_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    queue_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    return (1);
}

FT_TEST(test_ft_queue_lock_blocks_until_release,
        "ft_queue lock waits for mutex holders to release")
{
    ft_queue<int> queue_instance;
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, queue_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&queue_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_type::last_operation_error());
    return (1);
}
