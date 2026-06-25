#include "../test_internal.hpp"
#include "../../Modules/Template/set.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_set_enable_thread_safety_installs_mutex)
{
    ft_set<int> set_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.initialize());
    FT_ASSERT_EQ(0, set_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());
    FT_ASSERT(set_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    set_instance.insert(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());
    set_instance.insert(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    const int *found_value = set_instance.find(10);
    FT_ASSERT(found_value != ft_nullptr);
    FT_ASSERT_EQ(10, *found_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    FT_ASSERT(set_instance.size() == 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    set_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());
    FT_ASSERT(set_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    ft_bool lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, set_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_FALSE);
    set_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.destroy());
    return (1);
}

FT_TEST(test_ft_set_lock_blocks_until_release)
{
    ft_set<int> set_instance;
    ft_bool main_lock_acquired;
    std::atomic<ft_bool> ready;
    std::atomic<ft_bool> worker_succeeded;
    std::atomic<int64_t> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.initialize());
    FT_ASSERT_EQ(0, set_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());

    main_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, set_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == FT_TRUE);

    ready.store(FT_FALSE);
    worker_succeeded.store(FT_FALSE);
    wait_duration_ms.store(0);
    worker = std::thread([&set_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        ft_bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = FT_FALSE;
        ready.store(FT_TRUE);
        start_time = std::chrono::steady_clock::now();
        if (set_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(FT_FALSE);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        set_instance.unlock(worker_lock_acquired);
    });

    while (ready.load() == FT_FALSE)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    set_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    set_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_instance.destroy());
    return (1);
}
