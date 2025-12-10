#include "../../Template/pair.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_pair_enable_thread_safety_installs_mutex,
        "Pair installs optional mutex guards when requested")
{
    Pair<int, int> pair_instance;

    FT_ASSERT_EQ(0, pair_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());
    FT_ASSERT(pair_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    pair_instance.set_key(5);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());
    pair_instance.set_value(10);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    int retrieved_key = pair_instance.get_key();
    FT_ASSERT_EQ(5, retrieved_key);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    int retrieved_value = pair_instance.get_value();
    FT_ASSERT_EQ(10, retrieved_value);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    pair_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());
    FT_ASSERT(pair_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    bool lock_acquired = false;
    FT_ASSERT_EQ(0, pair_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == false);
    pair_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());
    return (1);
}

FT_TEST(test_pair_lock_blocks_until_release,
        "Pair lock waits for mutex holders to release")
{
    Pair<int, int> pair_instance;
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(0, pair_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, pair_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&pair_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (pair_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        pair_instance.unlock(worker_lock_acquired);
        pair_instance.set_value(42);
        worker_succeeded.store(true);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pair_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    int stored_value = pair_instance.get_value();
    FT_ASSERT_EQ(42, stored_value);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());

    pair_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, pair_instance.get_error());
    return (1);
}

FT_TEST(test_pair_copy_and_move_preserve_thread_safety,
        "Pair copies and moves propagate thread safety state")
{
    Pair<int, int> original;

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    original.set_key(7);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    original.set_value(21);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());

    Pair<int, int> copied(original);
    FT_ASSERT(copied.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copied.get_error());
    FT_ASSERT_EQ(7, copied.get_key());
    FT_ASSERT_EQ(21, copied.get_value());

    Pair<int, int> assigned;
    assigned = original;
    FT_ASSERT(assigned.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, assigned.get_error());
    FT_ASSERT_EQ(7, assigned.get_key());
    FT_ASSERT_EQ(21, assigned.get_value());

    Pair<int, int> moved(ft_move(original));
    FT_ASSERT(moved.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved.get_error());
    FT_ASSERT_EQ(7, moved.get_key());
    FT_ASSERT_EQ(21, moved.get_value());

    Pair<int, int> move_assigned;
    FT_ASSERT_EQ(0, move_assigned.enable_thread_safety());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, move_assigned.get_error());
    move_assigned.set_key(1);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, move_assigned.get_error());
    move_assigned.set_value(2);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, move_assigned.get_error());
    move_assigned = ft_move(moved);
    FT_ASSERT(move_assigned.is_thread_safe());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, move_assigned.get_error());
    FT_ASSERT_EQ(7, move_assigned.get_key());
    FT_ASSERT_EQ(21, move_assigned.get_value());

    return (1);
}
