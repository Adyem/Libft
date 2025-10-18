#include "../../Logger/logger.hpp"
#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include <cstdarg>
#include <atomic>
#include <chrono>
#include <thread>

static void    logger_enqueue_message(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ft_log_enqueue(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
    return ;
}

FT_TEST(test_logger_async_backpressure_metrics, "async logger reports queue drops and backlog")
{
    s_log_async_metrics metrics;
    size_t original_limit;

    ft_log_enable_async(false);
    original_limit = ft_log_get_async_queue_limit();
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_set_async_queue_limit(2);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_reset_async_metrics();
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    logger_enqueue_message("backpressure-1");
    logger_enqueue_message("backpressure-2");
    logger_enqueue_message("backpressure-3");
    logger_enqueue_message("backpressure-4");
    FT_ASSERT_EQ(0, ft_log_get_async_metrics(&metrics));
    FT_ASSERT(metrics.pending_messages == 2);
    FT_ASSERT(metrics.dropped_messages >= 2);
    FT_ASSERT(metrics.peak_pending_messages >= metrics.pending_messages);
    ft_log_enable_async(true);
    ft_log_enable_async(false);
    ft_log_set_async_queue_limit(original_limit);
    ft_log_reset_async_metrics();
    return (1);
}

FT_TEST(test_ft_log_set_level_updates_global_threshold, "ft_log_set_level updates g_level and clears errno")
{
    t_log_level previous_level;
    int         previous_errno_value;

    previous_level = g_level;
    previous_errno_value = ft_errno;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_log_set_level(LOG_LEVEL_ERROR);
    FT_ASSERT_EQ(LOG_LEVEL_ERROR, g_level);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    g_level = previous_level;
    ft_errno = previous_errno_value;
    return (1);
}

FT_TEST(test_ft_log_alloc_logging_without_global_logger, "ft_log_set_alloc_logging handles missing global logger")
{
    ft_logger *previous_logger;
    int         previous_errno_value;

    previous_logger = g_logger;
    FT_ASSERT(previous_logger == ft_nullptr);
    g_logger = ft_nullptr;
    previous_errno_value = ft_errno;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_log_set_alloc_logging(true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(false, ft_log_get_alloc_logging());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    g_logger = previous_logger;
    ft_errno = previous_errno_value;
    return (1);
}

FT_TEST(test_ft_log_global_helpers_forward_to_logger, "ft_log_* helpers forward to the active logger instance")
{
    ft_logger  *previous_logger;
    int         previous_errno_value;

    previous_logger = g_logger;
    FT_ASSERT(previous_logger == ft_nullptr);
    previous_errno_value = ft_errno;
    {
        ft_logger logger_instance;

        logger_instance.set_global();
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_log_set_alloc_logging(true);
        FT_ASSERT_EQ(true, logger_instance.get_alloc_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_ERR_INVALID_ARGUMENT;
        FT_ASSERT_EQ(true, ft_log_get_alloc_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_log_set_api_logging(true);
        FT_ASSERT_EQ(true, logger_instance.get_api_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_ERR_INVALID_ARGUMENT;
        FT_ASSERT_EQ(true, ft_log_get_api_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_log_set_alloc_logging(false);
        FT_ASSERT_EQ(false, logger_instance.get_alloc_logging());
        ft_log_set_api_logging(false);
        FT_ASSERT_EQ(false, logger_instance.get_api_logging());

        g_logger = previous_logger;
    }
    ft_errno = previous_errno_value;
    return (1);
}

FT_TEST(test_logger_log_field_prepare_thread_safety_initializes_mutex,
        "log_field_prepare_thread_safety installs mutex guard")
{
    s_log_field field;
    bool        lock_acquired;

    FT_ASSERT_EQ(0, log_field_prepare_thread_safety(&field));
    FT_ASSERT(field.thread_safe_enabled == true);
    FT_ASSERT(field.mutex != ft_nullptr);
    lock_acquired = false;
    FT_ASSERT_EQ(0, log_field_lock(&field, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    log_field_unlock(&field, lock_acquired);
    log_field_teardown_thread_safety(&field);
    return (1);
}

FT_TEST(test_logger_log_field_lock_blocks_until_released,
        "log_field_lock waits for prior holder to release the mutex")
{
    s_log_field          field;
    bool                 main_lock_acquired;
    std::atomic<bool>    ready;
    std::atomic<bool>    worker_failed;
    std::atomic<long long> wait_duration_ms;
    std::thread          worker;

    field.key = "field";
    FT_ASSERT_EQ(0, log_field_prepare_thread_safety(&field));
    main_lock_acquired = false;
    FT_ASSERT_EQ(0, log_field_lock(&field, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);
    ready.store(false);
    worker_failed.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&field, &ready, &worker_failed, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (log_field_lock(&field, &worker_lock_acquired) != 0)
        {
            worker_failed.store(true);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        if (!worker_lock_acquired)
            worker_failed.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        log_field_unlock(&field, worker_lock_acquired);
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    log_field_unlock(&field, main_lock_acquired);
    worker.join();
    FT_ASSERT(worker_failed.load() == false);
    FT_ASSERT(wait_duration_ms.load() >= 40);
    log_field_teardown_thread_safety(&field);
    return (1);
}

FT_TEST(test_logger_log_sink_prepare_thread_safety_initializes_mutex,
        "log_sink_prepare_thread_safety installs mutex guard")
{
    s_log_sink sink;
    bool       lock_acquired;

    FT_ASSERT_EQ(0, log_sink_prepare_thread_safety(&sink));
    FT_ASSERT(sink.thread_safe_enabled == true);
    FT_ASSERT(sink.mutex != ft_nullptr);
    lock_acquired = false;
    FT_ASSERT_EQ(0, log_sink_lock(&sink, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    log_sink_unlock(&sink, lock_acquired);
    log_sink_teardown_thread_safety(&sink);
    return (1);
}

FT_TEST(test_logger_log_sink_lock_blocks_until_release,
        "log_sink_lock waits for active sink usage to complete")
{
    s_log_sink           sink;
    bool                 main_lock_acquired;
    std::atomic<bool>    ready;
    std::atomic<bool>    worker_failed;
    std::atomic<long long> wait_duration_ms;
    std::thread          worker;

    FT_ASSERT_EQ(0, log_sink_prepare_thread_safety(&sink));
    main_lock_acquired = false;
    FT_ASSERT_EQ(0, log_sink_lock(&sink, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);
    ready.store(false);
    worker_failed.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&sink, &ready, &worker_failed, &wait_duration_ms]() {
        bool                                       worker_lock_acquired;
        std::chrono::steady_clock::time_point      start_time;
        std::chrono::steady_clock::time_point      end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (log_sink_lock(&sink, &worker_lock_acquired) != 0)
        {
            worker_failed.store(true);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        if (!worker_lock_acquired)
            worker_failed.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        log_sink_unlock(&sink, worker_lock_acquired);
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    log_sink_unlock(&sink, main_lock_acquired);
    worker.join();
    FT_ASSERT(worker_failed.load() == false);
    FT_ASSERT(wait_duration_ms.load() >= 40);
    log_sink_teardown_thread_safety(&sink);
    return (1);
}

FT_TEST(test_logger_async_metrics_lock_blocks_until_release,
        "log_async_metrics_lock protects concurrent metric reads")
{
    s_log_async_metrics   metrics;
    bool                  main_lock_acquired;
    std::atomic<bool>     ready;
    std::atomic<int>      worker_result;
    std::atomic<long long> wait_duration_ms;
    std::thread           worker;

    FT_ASSERT_EQ(0, log_async_metrics_prepare_thread_safety(&metrics));
    main_lock_acquired = false;
    FT_ASSERT_EQ(0, log_async_metrics_lock(&metrics, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);
    ready.store(false);
    worker_result.store(-2);
    wait_duration_ms.store(0);
    worker = std::thread([&metrics, &ready, &worker_result, &wait_duration_ms]() {
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        int result;

        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        result = ft_log_get_async_metrics(&metrics);
        end_time = std::chrono::steady_clock::now();
        worker_result.store(result);
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    log_async_metrics_unlock(&metrics, main_lock_acquired);
    worker.join();
    FT_ASSERT_EQ(0, worker_result.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);
    log_async_metrics_teardown_thread_safety(&metrics);
    return (1);
}

