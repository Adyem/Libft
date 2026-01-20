#include "../../Template/unordered_map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_unordered_map_iterator_enable_thread_safety_controls,
        "ft_unordered_map iterator exposes optional mutex guards")
{
    ft_unordered_map<int, int> map_instance;

    map_instance.insert(1, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());
    map_instance.insert(2, 20);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());

    ft_unordered_map<int, int>::iterator iterator_instance = map_instance.begin();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());

    FT_ASSERT_EQ(0, iterator_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());
    FT_ASSERT(iterator_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    bool lock_acquired = false;
    FT_ASSERT_EQ(0, iterator_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    iterator_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    FT_ASSERT_EQ(1, (*iterator_instance).first);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    iterator_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());
    return (1);
}

FT_TEST(test_ft_unordered_map_iterator_lock_blocks_until_release,
        "ft_unordered_map iterator lock waits for mutex holders to release")
{
    ft_unordered_map<int, int> map_instance;
    ft_unordered_map<int, int>::iterator iterator_instance(map_instance.begin());
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker;

    map_instance.insert(1, 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());

    iterator_instance = map_instance.begin();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());
    FT_ASSERT_EQ(0, iterator_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, iterator_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&iterator_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (iterator_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        iterator_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    iterator_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    iterator_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());
    return (1);
}

FT_TEST(test_ft_unordered_map_iterators_inherit_container_thread_safety,
        "ft_unordered_map propagates thread-safety toggles to iterators")
{
    ft_unordered_map<int, int> map_instance;

    FT_ASSERT_EQ(0, map_instance.enable_thread_safety());
    FT_ASSERT(map_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());

    map_instance.insert(42, 100);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());

    ft_unordered_map<int, int>::iterator iterator_instance = map_instance.begin();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());
    FT_ASSERT(iterator_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    bool iterator_lock_acquired = false;
    FT_ASSERT_EQ(0, iterator_instance.lock(&iterator_lock_acquired));
    FT_ASSERT(iterator_lock_acquired);
    iterator_instance.unlock(iterator_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());

    const ft_unordered_map<int, int>& const_map_ref = map_instance;
    ft_unordered_map<int, int>::const_iterator const_iterator_instance = const_map_ref.begin();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, const_map_ref.last_operation_error());
    FT_ASSERT(const_iterator_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, const_iterator_instance.get_error());

    bool const_lock_acquired = false;
    FT_ASSERT_EQ(0, const_iterator_instance.lock(&const_lock_acquired));
    FT_ASSERT(const_lock_acquired);
    const_iterator_instance.unlock(const_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, const_iterator_instance.get_error());

    iterator_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, iterator_instance.get_error());
    const_iterator_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, const_iterator_instance.get_error());
    map_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.last_operation_error());
    return (1);
}
