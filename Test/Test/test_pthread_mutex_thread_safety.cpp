#include "../../PThread/mutex.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_pt_mutex_state_guard_controls,
        "pt_mutex installs optional guard mutex for internal state")
{
    pt_mutex mutex_object;
    bool lock_acquired;

    FT_ASSERT(mutex_object.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());

    FT_ASSERT_EQ(0, mutex_object.enable_thread_safety());
    FT_ASSERT(mutex_object.is_thread_safe());
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, mutex_object.lock_state(&lock_acquired));
    FT_ASSERT(lock_acquired);
    mutex_object.unlock_state(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());

    mutex_object.disable_thread_safety();
    FT_ASSERT(mutex_object.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_state_guard_blocks_until_release,
        "pt_mutex state guard waits for prior holders to unlock")
{
    pt_mutex mutex_object;
    bool main_lock_acquired;
    std::atomic<bool> worker_started;
    std::atomic<bool> worker_locked;
    std::atomic<bool> worker_finished;
    std::thread worker_thread;

    FT_ASSERT_EQ(0, mutex_object.enable_thread_safety());
    FT_ASSERT(mutex_object.is_thread_safe());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, mutex_object.lock_state(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    worker_started.store(false);
    worker_locked.store(false);
    worker_finished.store(false);
    worker_thread = std::thread([&mutex_object, &worker_started, &worker_locked, &worker_finished]() {
        bool worker_lock_acquired;

        worker_lock_acquired = false;
        worker_started.store(true);
        if (mutex_object.lock_state(&worker_lock_acquired) != 0)
        {
            worker_locked.store(false);
            worker_finished.store(true);
            return ;
        }
        worker_locked.store(worker_lock_acquired);
        mutex_object.unlock_state(worker_lock_acquired);
        worker_finished.store(true);
    });

    while (!worker_started.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    FT_ASSERT(worker_locked.load() == false);

    mutex_object.unlock_state(main_lock_acquired);
    worker_thread.join();

    FT_ASSERT(worker_finished.load());
    FT_ASSERT(worker_locked.load());
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());

    mutex_object.disable_thread_safety();
    FT_ASSERT(mutex_object.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}
