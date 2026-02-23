#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include "../../SCMA/SCMA.hpp"
#include "../../SCMA/scma_internal.hpp"
#include "../../PThread/recursive_mutex.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

static void scma_mutex_failure_prepare(void)
{
    (void)scma_disable_thread_safety();
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    return ;
}

FT_TEST(test_scma_mutex_lock_failure_direct,
    "scma_mutex_lock returns -1 when the runtime recursive mutex lock fails")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(-1, scma_mutex_lock());
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_initialize_mutex_lock_failure,
    "scma_initialize returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_initialize(64));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_allocate_mutex_lock_failure,
    "scma_allocate returns invalid handle when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    scma_handle result = scma_allocate(16);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_handle invalid = scma_invalid_handle();
    FT_ASSERT_EQ(invalid.index, result.index);
    FT_ASSERT_EQ(invalid.generation, result.generation);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_free_mutex_lock_failure,
    "scma_free returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_free(scma_invalid_handle()));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_resize_mutex_lock_failure,
    "scma_resize returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_resize(scma_invalid_handle(), 32));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_shutdown_mutex_lock_failure,
    "scma_shutdown exits early when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    scma_shutdown();
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(1, scma_is_initialized());
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_is_initialized_mutex_lock_failure,
    "scma_is_initialized returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_is_initialized());
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_get_size_mutex_lock_failure,
    "scma_get_size returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    scma_handle handle = scma_allocate(8);
    if (handle.index == scma_invalid_handle().index
        && handle.generation == scma_invalid_handle().generation)
    {
        scma_shutdown();
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
        return (0);
    }
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_get_size(handle));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_read_mutex_lock_failure,
    "scma_read returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    scma_handle handle = scma_allocate(8);
    if (handle.index == scma_invalid_handle().index
        && handle.generation == scma_invalid_handle().generation)
    {
        scma_shutdown();
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
        return (0);
    }
    char buffer[8] = {0};
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_read(handle, 0, buffer, sizeof(buffer)));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_write_mutex_lock_failure,
    "scma_write returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    scma_handle handle = scma_allocate(8);
    if (handle.index == scma_invalid_handle().index
        && handle.generation == scma_invalid_handle().generation)
    {
        scma_shutdown();
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
        return (0);
    }
    const char source[8] = "scma";
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_write(handle, 0, source, sizeof(source)));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_get_stats_mutex_lock_failure,
    "scma_get_stats returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    scma_stats stats = {0, 0, 0};
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_get_stats(&stats));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_handle_is_valid_mutex_lock_failure,
    "scma_handle_is_valid returns 0 when the runtime mutex lock fails")
{
    scma_mutex_failure_prepare();
    FT_ASSERT_EQ(1, scma_initialize(64));
    scma_handle handle = scma_allocate(8);
    if (handle.index == scma_invalid_handle().index
        && handle.generation == scma_invalid_handle().generation)
    {
        scma_shutdown();
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
        return (0);
    }
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    FT_ASSERT_EQ(0, scma_handle_is_valid(handle));
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}
