#include "../test_internal.hpp"
#include "test_scma_shared.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_scma_thread_safety_toggle_cycle,
    "scma explicit thread safety enable and disable toggles runtime mutex state")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(false, scma_is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    FT_ASSERT_EQ(true, scma_is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    FT_ASSERT_EQ(true, scma_is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(false, scma_is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_scma_disable_thread_safety_reports_busy_while_locked,
    "scma disable thread safety reports busy when runtime mutex is in active use")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    FT_ASSERT_EQ(0, scma_mutex_lock());
    FT_ASSERT_EQ(1U, scma_mutex_lock_count());
    FT_ASSERT_EQ(FT_ERR_THREAD_BUSY, scma_disable_thread_safety());
    FT_ASSERT_EQ(0, scma_mutex_unlock());
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_mutex_close_unwinds_recursive_depth,
    "scma mutex close releases all recursive depth levels")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_enable_thread_safety());
    FT_ASSERT_EQ(0, scma_mutex_lock());
    FT_ASSERT_EQ(0, scma_mutex_lock());
    FT_ASSERT_EQ(2U, scma_mutex_lock_count());
    FT_ASSERT_EQ(0, scma_mutex_close());
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}

FT_TEST(test_scma_mutex_api_rejects_usage_without_thread_safety,
    "scma lock API rejects direct lock and unlock when thread safety is disabled")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(-1, scma_mutex_lock());
    FT_ASSERT_EQ(-1, scma_mutex_unlock());
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    return (1);
}

FT_TEST(test_scma_initialize_enables_thread_safety,
    "scma initialize enables runtime thread safety automatically")
{
    scma_test_reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    FT_ASSERT_EQ(false, scma_is_thread_safe_enabled());
    FT_ASSERT_EQ(1, scma_initialize(64));
    FT_ASSERT_EQ(true, scma_is_thread_safe_enabled());
    scma_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_disable_thread_safety());
    return (1);
}
