#include "../../Template/matrix.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_matrix_enable_thread_safety_installs_mutex,
        "ft_matrix installs optional mutex guards when requested")
{
    ft_matrix<int> matrix_instance;
    bool           lock_acquired;

    FT_ASSERT_EQ(0, matrix_instance.enable_thread_safety());
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    FT_ASSERT(matrix_instance.is_thread_safe());
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    FT_ASSERT(matrix_instance.init(2, 2));
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    matrix_instance.at(0, 0) = 1;
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    matrix_instance.at(1, 1) = 9;
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, matrix_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    matrix_instance.at(0, 1) = 3;
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    matrix_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    matrix_instance.disable_thread_safety();
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    FT_ASSERT(matrix_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    lock_acquired = false;
    FT_ASSERT_EQ(0, matrix_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    matrix_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    return (1);
}

FT_TEST(test_ft_matrix_lock_blocks_until_release,
        "ft_matrix lock waits for mutex holders to release")
{
    ft_matrix<int>                  matrix_instance;
    bool                            main_lock_acquired;
    std::atomic<bool>               ready;
    std::atomic<bool>               worker_succeeded;
    std::atomic<long long>          wait_duration_ms;
    std::thread                     worker;

    FT_ASSERT_EQ(0, matrix_instance.enable_thread_safety());
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    FT_ASSERT(matrix_instance.init(3, 3));
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, matrix_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);

    worker = std::thread([&matrix_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool                                   worker_lock_acquired;
        std::chrono::steady_clock::time_point  start_time;
        std::chrono::steady_clock::time_point  end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (matrix_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        matrix_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    matrix_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    matrix_instance.disable_thread_safety();
    FT_ASSERT_EQ(ER_SUCCESS, matrix_instance.get_error());
    return (1);
}
