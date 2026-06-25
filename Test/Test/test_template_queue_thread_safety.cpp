#include "../test_internal.hpp"
#include "../../Modules/Template/queue.hpp"
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

using queue_type = ft_queue<int>;

static int queue_thread_safety_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_queue_get_error_returned = FT_FALSE;
static int32_t g_queue_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_queue_get_error_str_returned = FT_FALSE;
static const char *g_queue_get_error_str_result = ft_nullptr;

static void queue_get_error_uninitialised_operation(void)
{
    queue_type queue_value;

    g_queue_get_error_result = queue_value.get_error();
    g_queue_get_error_returned = FT_TRUE;
    return ;
}

static void queue_get_error_str_uninitialised_operation(void)
{
    queue_type queue_value;

    g_queue_get_error_str_result = queue_value.get_error_str();
    g_queue_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_queue_enable_thread_safety_initializes_mutex)
{
    queue_type queue_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT(queue_instance.is_thread_safe());

    queue_instance.enqueue(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(false, queue_instance.empty());

    int value = queue_instance.dequeue();
    FT_ASSERT_EQ(7, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT(queue_instance.empty());

    queue_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT(queue_instance.is_thread_safe() == false);

    ft_bool lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, queue_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired == FT_FALSE);
    queue_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}

FT_TEST(test_ft_queue_lock_blocks_until_release)
{
    ft_queue<int> queue_instance;
    ft_bool main_lock_acquired;
    std::atomic<ft_bool> ready;
    std::atomic<ft_bool> worker_succeeded;
    std::atomic<int64_t> wait_duration_ms;
    std::thread worker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    FT_ASSERT_EQ(0, queue_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());

    main_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, queue_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == FT_TRUE);

    ready.store(FT_FALSE);
    worker_succeeded.store(FT_FALSE);
    wait_duration_ms.store(0);
    worker = std::thread([&queue_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        ft_bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = FT_FALSE;
        ready.store(FT_TRUE);
        start_time = std::chrono::steady_clock::now();
        if (queue_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(FT_FALSE);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        queue_instance.unlock(worker_lock_acquired);
    });

    while (ready.load() == FT_FALSE)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue_instance.unlock(main_lock_acquired);

    worker.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    queue_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    return (1);
}

FT_TEST(test_ft_queue_error_queries_follow_lifecycle_contract)
{
    queue_type queue_instance;

    g_queue_get_error_returned = FT_FALSE;
    g_queue_get_error_result = FT_ERR_SUCCESS;
    g_queue_get_error_str_returned = FT_FALSE;
    g_queue_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, queue_thread_safety_expect_sigabrt(
        queue_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_queue_get_error_returned);
    FT_ASSERT_EQ(1, queue_thread_safety_expect_sigabrt(
        queue_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_queue_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue_instance.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(queue_instance.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
