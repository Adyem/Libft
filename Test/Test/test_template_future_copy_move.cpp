#include "../test_internal.hpp"
#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_future_initialize_copy_from_uninitialised_rebuilds_mutex)
{
    ft_promise<int> promise;
    ft_future<int> original_future(promise);
    ft_future<int> copied_future;
    ft_bool original_lock_acquired;
    ft_bool copy_lock_acquired;

    promise.set_value(19);
    original_lock_acquired = FT_FALSE;
    copy_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.initialize());
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT_EQ(0, copied_future.initialize(original_future));
    FT_ASSERT(original_future.is_thread_safe());
    FT_ASSERT(copied_future.is_thread_safe());
    FT_ASSERT_EQ(0, original_future.lock(&original_lock_acquired));
    FT_ASSERT(original_lock_acquired == FT_TRUE);
    FT_ASSERT_EQ(0, copied_future.lock(&copy_lock_acquired));
    FT_ASSERT(copy_lock_acquired == FT_TRUE);
    copied_future.unlock(copy_lock_acquired);
    original_future.unlock(original_lock_acquired);
    FT_ASSERT_EQ(19, copied_future.get());
    FT_ASSERT_EQ(19, original_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_initialize_copy_recreates_mutex)
{
    ft_promise<int> source_promise;
    ft_future<int> source_future(source_promise);
    ft_promise<int> destination_promise;
    ft_future<int> destination_future(destination_promise);
    ft_bool source_lock_acquired;
    ft_bool destination_lock_acquired;

    source_promise.set_value(23);
    source_lock_acquired = FT_FALSE;
    destination_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.initialize());
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.initialize(source_future));
    FT_ASSERT(destination_future.is_thread_safe());
    FT_ASSERT(source_future.is_thread_safe());
    FT_ASSERT_EQ(0, source_future.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired == FT_TRUE);
    FT_ASSERT_EQ(0, destination_future.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired == FT_TRUE);
    destination_future.unlock(destination_lock_acquired);
    source_future.unlock(source_lock_acquired);
    FT_ASSERT_EQ(23, destination_future.get());
    FT_ASSERT_EQ(23, source_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_initialize_move_from_uninitialised_rebuilds_mutex)
{
    ft_promise<int> promise;
    ft_future<int> original_future(promise);
    ft_future<int> moved_future;
    ft_bool moved_lock_acquired;

    promise.set_value(31);
    moved_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.initialize());
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe());
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(original_future)));
    FT_ASSERT(moved_future.is_thread_safe());
    FT_ASSERT_EQ(0, moved_future.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired == FT_TRUE);
    moved_future.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(31, moved_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_initialize_move_rebuilds_mutex)
{
    ft_promise<int> source_promise;
    ft_future<int> source_future(source_promise);
    ft_promise<int> destination_promise;
    ft_future<int> destination_future(destination_promise);
    ft_bool destination_lock_acquired;

    source_promise.set_value(47);
    destination_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.initialize());
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.initialize(ft_move(source_future)));
    FT_ASSERT(destination_future.is_thread_safe());
    FT_ASSERT_EQ(0, destination_future.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired == FT_TRUE);
    destination_future.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(47, destination_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_void_move_rebuilds_mutex)
{
    ft_promise<void> promise;
    ft_future<void> original_future(promise);
    ft_future<void> moved_future;
    ft_bool moved_lock_acquired;

    moved_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.initialize());
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe());
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(original_future)));
    FT_ASSERT(moved_future.is_thread_safe());
    FT_ASSERT_EQ(0, moved_future.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired == FT_TRUE);
    moved_future.unlock(moved_lock_acquired);
    promise.set_value();
    moved_future.get();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_future.destroy());
    return (1);
}
