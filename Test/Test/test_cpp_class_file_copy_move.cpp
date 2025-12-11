#define private public
#define protected public
#include "../../CPP_class/class_file.hpp"
#undef private
#undef protected
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <unistd.h>

static int create_pipe(int *read_end, int *write_end)
{
    int descriptors[2];
    int result;

    result = pipe(descriptors);
    if (result != 0)
        return (FT_ERR_INVALID_HANDLE);
    *read_end = descriptors[0];
    *write_end = descriptors[1];
    return (FT_ERR_SUCCESSS);
}

static void fill_pipe(int write_end, const char *data)
{
    const char *cursor;

    cursor = data;
    while (*cursor != '\0')
    {
        su_write(write_end, cursor, 1);
        cursor++;
    }
    return ;
}

static long measure_read_duration(ft_file &file, char &character)
{
    char buffer[2];
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    ssize_t bytes_read;

    buffer[0] = '\0';
    buffer[1] = '\0';
    start_time = std::chrono::steady_clock::now();
    bytes_read = file.read(buffer, 1);
    end_time = std::chrono::steady_clock::now();
    if (bytes_read == 1)
        character = buffer[0];
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

FT_TEST(test_ft_file_move_constructor_mutex_is_fresh,
        "ft_file move constructor initializes an independent mutex")
{
    int read_end;
    int write_end;
    ft_file source;
    ft_file moved;
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    char character;
    long duration_ms;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&read_end, &write_end));
    fill_pipe(write_end, "m");
    su_close(write_end);
    source = ft_file(read_end);
    moved = ft_file(ft_move(source));
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
    duration_ms = measure_read_duration(moved, character);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('m', character);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}

FT_TEST(test_ft_file_move_constructor_releases_source_descriptor,
        "ft_file move constructor clears the moved-from descriptor")
{
    int read_end;
    int write_end;
    ft_file source;
    ft_file moved;
    char character;
    long duration_ms;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&read_end, &write_end));
    fill_pipe(write_end, "r");
    su_close(write_end);
    source = ft_file(read_end);
    moved = ft_file(ft_move(source));
    duration_ms = measure_read_duration(moved, character);
    moved.close();
    FT_ASSERT_EQ(-1, source._fd);
    FT_ASSERT_EQ(read_end, moved._fd);
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('r', character);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}

FT_TEST(test_ft_file_move_assignment_mutex_is_fresh,
        "ft_file move assignment initializes an independent mutex")
{
    int source_read_end;
    int source_write_end;
    int target_read_end;
    int target_write_end;
    ft_file source;
    ft_file target;
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    char character;
    long duration_ms;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&source_read_end, &source_write_end));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&target_read_end, &target_write_end));
    fill_pipe(source_write_end, "a");
    su_close(source_write_end);
    su_close(target_write_end);
    source = ft_file(source_read_end);
    target = ft_file(target_read_end);
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
    FT_ASSERT_EQ('a', character);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target.get_error());
    return (1);
}

FT_TEST(test_ft_file_move_assignment_resets_source_descriptor,
        "ft_file move assignment clears the moved-from descriptor")
{
    int source_read_end;
    int source_write_end;
    int target_read_end;
    int target_write_end;
    ft_file source;
    ft_file target;
    char character;
    long duration_ms;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&source_read_end, &source_write_end));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, create_pipe(&target_read_end, &target_write_end));
    fill_pipe(source_write_end, "s");
    su_close(source_write_end);
    su_close(target_write_end);
    source = ft_file(source_read_end);
    target = ft_file(target_read_end);
    target = ft_move(source);
    duration_ms = measure_read_duration(target, character);
    target.close();
    FT_ASSERT_EQ(-1, source._fd);
    FT_ASSERT_EQ(source_read_end, target._fd);
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ('s', character);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target.get_error());
    return (1);
}
