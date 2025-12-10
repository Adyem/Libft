#include "../../Template/unique_ptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_uniqueptr_enable_thread_safety_installs_mutex,
        "ft_uniqueptr installs optional mutex guards when requested")
{
    ft_uniqueptr<int> unique_pointer(new int(17));
    bool              lock_acquired;

    FT_ASSERT_EQ(0, unique_pointer.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());
    FT_ASSERT(unique_pointer.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, unique_pointer.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    FT_ASSERT(unique_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());
    unique_pointer.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());

    unique_pointer.disable_thread_safety();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());
    FT_ASSERT(unique_pointer.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, unique_pointer.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    unique_pointer.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());
    return (1);
}

FT_TEST(test_ft_uniqueptr_lock_blocks_until_release,
        "ft_uniqueptr lock waits for mutex holders to release")
{
    ft_uniqueptr<int>            unique_pointer(new int(42));
    bool                         main_lock_acquired;
    std::atomic<bool>            ready;
    std::atomic<bool>            worker_succeeded;
    std::atomic<long long>       wait_duration_ms;
    std::thread                  worker;

    FT_ASSERT_EQ(0, unique_pointer.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, unique_pointer.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);

    worker = std::thread([&unique_pointer, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool                                   worker_lock_acquired;
        std::chrono::steady_clock::time_point  start_time;
        std::chrono::steady_clock::time_point  end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (unique_pointer.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        unique_pointer.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    unique_pointer.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 10);

    unique_pointer.disable_thread_safety();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, unique_pointer.get_error());
    return (1);
}
