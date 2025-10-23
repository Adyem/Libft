#include <atomic>
#include <chrono>
#include <cstring>
#include <thread>

#include "../../CMA/cma_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_cma_metadata_allocate_block_prepares_thread_safety,
        "cma_metadata_allocate_block installs block mutex protection")
{
    Block *block;
    bool lock_acquired;

    ft_errno = ER_SUCCESS;
    block = cma_metadata_allocate_block();
    FT_ASSERT(block != ft_nullptr);
    FT_ASSERT(block->thread_safe_enabled == true);
    FT_ASSERT(block->mutex != ft_nullptr);
    lock_acquired = false;
    FT_ASSERT_EQ(0, cma_block_lock(block, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    cma_block_unlock(block, lock_acquired);
    cma_metadata_release_block(block);
    return (1);
}

FT_TEST(test_cma_block_lock_waits_for_existing_holder,
        "cma_block_lock waits until the current owner releases the mutex")
{
    Block *block;
    std::atomic<bool> ready;
    std::thread worker;
    bool lock_acquired;
    std::atomic<bool> worker_failed;

    block = cma_metadata_allocate_block();
    if (block == ft_nullptr)
        return (0);
    ready.store(false);
    worker_failed.store(false);
    worker = std::thread([block, &ready, &worker_failed]() {
        bool worker_lock_acquired;

        worker_lock_acquired = false;
        if (cma_block_lock(block, &worker_lock_acquired) != 0)
        {
            worker_failed.store(true);
            ready.store(true);
            return ;
        }
        if (!worker_lock_acquired)
        {
            worker_failed.store(true);
            ready.store(true);
            return ;
        }
        ready.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cma_block_unlock(block, worker_lock_acquired);
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    long long elapsed_ms;

    int test_failed;
    const char *failure_expression;
    int failure_line;
    int lock_result;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    lock_acquired = false;
    start_time = std::chrono::steady_clock::now();
    lock_result = cma_block_lock(block, &lock_acquired);
    end_time = std::chrono::steady_clock::now();
    if (lock_result != 0 && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "lock_result == 0";
        failure_line = __LINE__;
    }
    if (lock_result == 0)
    {
        if (lock_acquired != true && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "lock_acquired == true";
            failure_line = __LINE__;
        }
        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
        if (elapsed_ms < 40 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "elapsed_ms >= 40";
            failure_line = __LINE__;
        }
        cma_block_unlock(block, lock_acquired);
    }
    worker.join();
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        cma_metadata_release_block(block);
        return (0);
    }
    FT_ASSERT(worker_failed.load() == false);
    cma_metadata_release_block(block);
    return (1);
}

FT_TEST(test_cma_page_prepare_thread_safety_initializes_mutex,
        "cma_page_prepare_thread_safety creates mutex guard")
{
    Page page;
    bool lock_acquired;

    std::memset(&page, 0, sizeof(Page));
    FT_ASSERT_EQ(0, cma_page_prepare_thread_safety(&page));
    FT_ASSERT(page.thread_safe_enabled == true);
    FT_ASSERT(page.mutex != ft_nullptr);
    lock_acquired = false;
    FT_ASSERT_EQ(0, cma_page_lock(&page, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    cma_page_unlock(&page, lock_acquired);
    cma_page_teardown_thread_safety(&page);
    return (1);
}

FT_TEST(test_cma_page_lock_blocks_until_unlocked,
        "cma_page_lock waits for concurrent holders to release")
{
    Page page;
    std::atomic<bool> ready;
    std::thread worker;
    bool lock_acquired;
    std::atomic<bool> worker_failed;

    std::memset(&page, 0, sizeof(Page));
    if (cma_page_prepare_thread_safety(&page) != 0)
        return (0);
    ready.store(false);
    worker_failed.store(false);
    worker = std::thread([&page, &ready, &worker_failed]() {
        bool worker_lock_acquired;

        worker_lock_acquired = false;
        if (cma_page_lock(&page, &worker_lock_acquired) != 0)
        {
            worker_failed.store(true);
            ready.store(true);
            return ;
        }
        if (!worker_lock_acquired)
        {
            worker_failed.store(true);
            ready.store(true);
            return ;
        }
        ready.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cma_page_unlock(&page, worker_lock_acquired);
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    long long elapsed_ms;

    int test_failed;
    const char *failure_expression;
    int failure_line;
    int lock_result;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    lock_acquired = false;
    start_time = std::chrono::steady_clock::now();
    lock_result = cma_page_lock(&page, &lock_acquired);
    end_time = std::chrono::steady_clock::now();
    if (lock_result != 0 && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "lock_result == 0";
        failure_line = __LINE__;
    }
    if (lock_result == 0)
    {
        if (lock_acquired != true && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "lock_acquired == true";
            failure_line = __LINE__;
        }
        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
        if (elapsed_ms < 40 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "elapsed_ms >= 40";
            failure_line = __LINE__;
        }
        cma_page_unlock(&page, lock_acquired);
    }
    worker.join();
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        cma_page_teardown_thread_safety(&page);
        return (0);
    }
    FT_ASSERT(worker_failed.load() == false);
    cma_page_teardown_thread_safety(&page);
    return (1);
}

