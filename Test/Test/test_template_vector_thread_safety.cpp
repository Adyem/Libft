#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_vector_enable_thread_safety_installs_mutex,
        "ft_vector installs optional mutex guards when requested")
{
    ft_vector<int> vector_instance;
    bool           lock_acquired;

    FT_ASSERT_EQ(0, vector_instance.enable_thread_safety());
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    FT_ASSERT(vector_instance.is_thread_safe());
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());

    vector_instance.push_back(5);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    vector_instance.push_back(7);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    FT_ASSERT_EQ(2u, vector_instance.size());
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, vector_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    vector_instance.push_back(11);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    vector_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());

    vector_instance.disable_thread_safety();
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    FT_ASSERT(vector_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, vector_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    vector_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    return (1);
}

FT_TEST(test_ft_vector_lock_blocks_until_release,
        "ft_vector lock waits for mutex holders to release")
{
    ft_vector<int>                  vector_instance;
    bool                            main_lock_acquired;
    std::atomic<bool>               ready;
    std::atomic<bool>               worker_succeeded;
    std::atomic<long long>          wait_duration_ms;
    std::thread                     worker;

    FT_ASSERT_EQ(0, vector_instance.enable_thread_safety());
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, vector_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);

    worker = std::thread([&vector_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool                                   worker_lock_acquired;
        std::chrono::steady_clock::time_point  start_time;
        std::chrono::steady_clock::time_point  end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (vector_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        vector_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    vector_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    vector_instance.disable_thread_safety();
    FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
    return (1);
}
