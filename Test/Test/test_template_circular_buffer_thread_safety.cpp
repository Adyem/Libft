#include "../test_internal.hpp"
#include "../../Modules/Template/circular_buffer.hpp"
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

FT_TEST(test_ft_circular_buffer_enable_thread_safety_installs_mutex)
{
    ft_circular_buffer<int> buffer_instance(3);
    ft_bool                 lock_acquired;
    int                     popped_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.initialize());
    FT_ASSERT_EQ(0, buffer_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    FT_ASSERT(buffer_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    buffer_instance.push(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    FT_ASSERT_EQ(2u, buffer_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, buffer_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_TRUE);
    buffer_instance.push(30);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    popped_value = buffer_instance.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    FT_ASSERT_EQ(10, popped_value);

    buffer_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    FT_ASSERT(buffer_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, buffer_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_FALSE);
    buffer_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_lock_blocks_until_release)
{
    ft_circular_buffer<int>      buffer_instance(2);
    ft_bool                      main_lock_acquired;
    std::atomic<bool>            ready;
    std::atomic<bool>            worker_succeeded;
    std::atomic<long long>       wait_duration_ms;
    std::thread                  worker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.initialize());
    FT_ASSERT_EQ(0, buffer_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    main_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, buffer_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == FT_TRUE);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);

    worker = std::thread([&buffer_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        ft_bool                                worker_lock_acquired;
        std::chrono::steady_clock::time_point  start_time;
        std::chrono::steady_clock::time_point  end_time;

        worker_lock_acquired = FT_FALSE;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (buffer_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        buffer_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    buffer_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    buffer_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    return (1);
}
