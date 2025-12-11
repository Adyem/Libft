#include "../../Template/thread_pool.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_thread_pool_enable_thread_safety_controls,
        "ft_thread_pool installs optional mutex guards when requested")
{
    ft_thread_pool pool_instance(2, 4);
    std::atomic<int> task_count;
    bool lock_acquired;
    int task_index;

    task_count.store(0);
    FT_ASSERT_EQ(0, pool_instance.enable_thread_safety());
    FT_ASSERT(pool_instance.is_thread_safe());
    lock_acquired = false;
    FT_ASSERT_EQ(0, pool_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    pool_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pool_instance.get_error());

    task_index = 0;
    while (task_index < 4)
    {
        pool_instance.submit([&task_count]() { task_count.fetch_add(1); });
        ++task_index;
    }

    pool_instance.wait();
    FT_ASSERT_EQ(4, task_count.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pool_instance.get_error());

    pool_instance.disable_thread_safety();
    FT_ASSERT(pool_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pool_instance.get_error());

    pool_instance.destroy();
    return (1);
}

FT_TEST(test_ft_thread_pool_lock_blocks_until_release,
        "ft_thread_pool lock waits for mutex holders to release")
{
    ft_thread_pool pool_instance(1, 2);
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker_thread;

    FT_ASSERT_EQ(0, pool_instance.enable_thread_safety());
    FT_ASSERT(pool_instance.is_thread_safe());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, pool_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker_thread = std::thread([&pool_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (pool_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        pool_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pool_instance.unlock(main_lock_acquired);

    worker_thread.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    pool_instance.disable_thread_safety();
    FT_ASSERT(pool_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pool_instance.get_error());

    pool_instance.destroy();
    return (1);
}
