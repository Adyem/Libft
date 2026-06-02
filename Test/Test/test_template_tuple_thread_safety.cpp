#include "../test_internal.hpp"
#include "../../Modules/Template/tuple.hpp"
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

using tuple_thread_type = ft_tuple<int, int>;

FT_TEST(test_ft_tuple_enable_thread_safety_installs_mutex)
{
    ft_tuple<int, int> tuple_instance(1, 2);
    ft_bool               lock_acquired;

    FT_ASSERT_EQ(0, tuple_instance.enable_thread_safety());
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    FT_ASSERT(tuple_instance.is_thread_safe());
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);

    FT_ASSERT_EQ(1, tuple_instance.get<0>());
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    FT_ASSERT_EQ(2, tuple_instance.get<1>());
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, tuple_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    tuple_instance.reset();
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    tuple_instance.unlock(lock_acquired);
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);

    tuple_instance.disable_thread_safety();
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    FT_ASSERT(tuple_instance.is_thread_safe() == FT_FALSE);
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, tuple_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_FALSE);
    tuple_instance.unlock(lock_acquired);
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    return (1);
}

FT_TEST(test_ft_tuple_lock_blocks_until_release)
{
    ft_tuple<int, int>            tuple_instance(3, 4);
    ft_bool                          main_lock_acquired;
    std::atomic<ft_bool>             ready;
    std::atomic<ft_bool>             worker_succeeded;
    std::atomic<long long>        wait_duration_ms;
    std::thread                   worker;

    FT_ASSERT_EQ(0, tuple_instance.enable_thread_safety());
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);

    main_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, tuple_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    ready.store(FT_FALSE);
    worker_succeeded.store(FT_FALSE);
    wait_duration_ms.store(0);

    worker = std::thread([&tuple_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        ft_bool                                   worker_lock_acquired;
        std::chrono::steady_clock::time_point  start_time;
        std::chrono::steady_clock::time_point  end_time;

        worker_lock_acquired = FT_FALSE;
        ready.store(FT_TRUE);
        start_time = std::chrono::steady_clock::now();
        if (tuple_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(FT_FALSE);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        tuple_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    tuple_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    tuple_instance.disable_thread_safety();
    FT_ASSERT(tuple_instance.get_error() == FT_ERR_SUCCESS);
    return (1);
}
