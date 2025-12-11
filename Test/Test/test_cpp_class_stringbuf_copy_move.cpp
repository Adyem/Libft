#define private public
#define protected public
#include "../../CPP_class/class_stringbuf.hpp"
#undef private
#undef protected
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/pthread.hpp"
#include <atomic>
#include <chrono>
#include <thread>

static long measure_read_duration(ft_stringbuf &buffer, char &character)
{
    char storage[2];
    std::size_t bytes_read;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    storage[0] = '\0';
    storage[1] = '\0';
    start_time = std::chrono::steady_clock::now();
    bytes_read = buffer.read(storage, 1);
    end_time = std::chrono::steady_clock::now();
    if (bytes_read == 1)
        character = storage[0];
    else
        character = '\0';
    return (std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

static void wait_until_true(std::atomic<bool> &flag)
{
    while (!flag.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return ;
}

FT_TEST(test_ft_stringbuf_copy_constructor_mutex_is_fresh,
        "ft_stringbuf copy constructor uses a fresh mutex")
{
    ft_string source("mutexcopy");
    ft_stringbuf original(source);
    ft_stringbuf copied(original);
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    long duration_ms;
    char character;

    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(FT_ERR_SUCCESSS);
    locker_thread = std::thread([&original, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(original._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != FT_ERR_SUCCESSS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        original._mutex.unlock(THREAD_ID);
    });
    wait_until_true(lock_acquired);
    duration_ms = measure_read_duration(copied, character);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('m', character);
    FT_ASSERT_EQ(false, copied.is_bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copied.get_error());
    return (1);
}

FT_TEST(test_ft_stringbuf_move_constructor_mutex_is_fresh,
        "ft_stringbuf move constructor uses a fresh mutex")
{
    ft_string source("movedata");
    ft_stringbuf original(source);
    ft_stringbuf moved(ft_move(original));
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    long duration_ms;
    char character;

    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(FT_ERR_SUCCESSS);
    locker_thread = std::thread([&original, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(original._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != FT_ERR_SUCCESSS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        original._mutex.unlock(THREAD_ID);
    });
    wait_until_true(lock_acquired);
    duration_ms = measure_read_duration(moved, character);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('m', character);
    FT_ASSERT_EQ(false, moved.is_bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}

FT_TEST(test_ft_stringbuf_copy_assignment_mutex_is_fresh,
        "ft_stringbuf copy assignment uses a fresh mutex")
{
    ft_string source_data("assigned");
    ft_string target_data("initial");
    ft_stringbuf source(source_data);
    ft_stringbuf target(target_data);
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    long duration_ms;
    char character;

    target = source;
    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(FT_ERR_SUCCESSS);
    locker_thread = std::thread([&source, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(source._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != FT_ERR_SUCCESSS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        source._mutex.unlock(THREAD_ID);
    });
    wait_until_true(lock_acquired);
    duration_ms = measure_read_duration(target, character);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('a', character);
    FT_ASSERT_EQ(false, target.is_bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target.get_error());
    return (1);
}

FT_TEST(test_ft_stringbuf_move_assignment_mutex_is_fresh,
        "ft_stringbuf move assignment uses a fresh mutex")
{
    ft_string source_data("movelock");
    ft_string target_data("before");
    ft_stringbuf source(source_data);
    ft_stringbuf target(target_data);
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    long duration_ms;
    char character;

    target = ft_move(source);
    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(FT_ERR_SUCCESSS);
    locker_thread = std::thread([&source, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(source._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != FT_ERR_SUCCESSS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        source._mutex.unlock(THREAD_ID);
    });
    wait_until_true(lock_acquired);
    duration_ms = measure_read_duration(target, character);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('m', character);
    FT_ASSERT_EQ(false, target.is_bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target.get_error());
    return (1);
}
