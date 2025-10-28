#include "../../API/api_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_api_connection_pool_handle_enable_lock_controls,
        "api_connection_pool_handle installs optional mutex protection")
{
    api_connection_pool_handle handle;
    bool lock_acquired;

    FT_ASSERT(handle.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, handle.get_error());
    FT_ASSERT_EQ(0, handle.enable_thread_safety());
    FT_ASSERT(handle.is_thread_safe());

    lock_acquired = false;
    FT_ASSERT_EQ(0, handle.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    handle.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, handle.get_error());

    handle.disable_thread_safety();
    FT_ASSERT(handle.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, handle.get_error());
    return (1);
}

FT_TEST(test_api_connection_pool_handle_lock_guard_waits_for_mutex,
        "api_connection_pool_handle guard blocks until mutex released")
{
    api_connection_pool_handle handle;
    bool main_lock_acquired;
    std::atomic<bool> worker_started;
    std::atomic<bool> worker_locked;
    std::atomic<bool> worker_finished;
    std::thread worker_thread;

    FT_ASSERT_EQ(0, handle.enable_thread_safety());
    FT_ASSERT(handle.is_thread_safe());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, handle.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    worker_started.store(false);
    worker_locked.store(false);
    worker_finished.store(false);
    worker_thread = std::thread([&handle, &worker_started, &worker_locked, &worker_finished]() {
        worker_started.store(true);
        api_connection_pool_handle_lock_guard guard(handle);

        if (guard.is_locked())
        {
            worker_locked.store(true);
            handle.should_store = false;
        }
        else
            worker_locked.store(false);
        worker_finished.store(true);
    });

    while (!worker_started.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    FT_ASSERT(worker_locked.load() == false);

    handle.unlock(main_lock_acquired);
    worker_thread.join();

    FT_ASSERT(worker_finished.load());
    FT_ASSERT(worker_locked.load());
    FT_ASSERT_EQ(ER_SUCCESS, handle.get_error());

    handle.disable_thread_safety();
    FT_ASSERT(handle.is_thread_safe() == false);
    return (1);
}

