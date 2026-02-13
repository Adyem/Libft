#include "../test_internal.hpp"
#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_future_initialize_copy_from_uninitialized_rebuilds_mutex,
        "ft_future initialize(copy) from uninitialized rebuilds mutex for shared promise")
{
    ft_promise<int> promise;
    ft_future<int> original_future(promise);
    ft_future<int> copied_future;
    bool original_lock_acquired;
    bool copy_lock_acquired;

    promise.set_value(19);
    original_lock_acquired = false;
    copy_lock_acquired = false;
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT_EQ(0, copied_future.initialize(original_future));
    FT_ASSERT(original_future.is_thread_safe_enabled());
    FT_ASSERT(copied_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, original_future.lock(&original_lock_acquired));
    FT_ASSERT(original_lock_acquired);
    FT_ASSERT_EQ(0, copied_future.lock(&copy_lock_acquired));
    FT_ASSERT(copy_lock_acquired);
    copied_future.unlock(copy_lock_acquired);
    original_future.unlock(original_lock_acquired);
    FT_ASSERT_EQ(19, copied_future.get());
    FT_ASSERT_EQ(19, original_future.get());
    return (1);
}

FT_TEST(test_ft_future_initialize_copy_recreates_mutex,
        "ft_future initialize(copy) reinstates mutex for shared state")
{
    ft_promise<int> source_promise;
    ft_future<int> source_future(source_promise);
    ft_promise<int> destination_promise;
    ft_future<int> destination_future(destination_promise);
    bool source_lock_acquired;
    bool destination_lock_acquired;

    source_promise.set_value(23);
    source_lock_acquired = false;
    destination_lock_acquired = false;
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.initialize(source_future));
    FT_ASSERT(destination_future.is_thread_safe_enabled());
    FT_ASSERT(source_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, source_future.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, destination_future.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    destination_future.unlock(destination_lock_acquired);
    source_future.unlock(source_lock_acquired);
    FT_ASSERT_EQ(23, destination_future.get());
    FT_ASSERT_EQ(23, source_future.get());
    return (1);
}

FT_TEST(test_ft_future_initialize_move_from_uninitialized_rebuilds_mutex,
        "ft_future initialize(move) from uninitialized recreates mutex and keeps moved-from reusable")
{
    ft_promise<int> promise;
    ft_future<int> original_future(promise);
    ft_future<int> moved_future;
    bool moved_lock_acquired;
    bool source_lock_acquired;

    promise.set_value(31);
    moved_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(original_future)));
    FT_ASSERT(moved_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, moved_future.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    moved_future.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(31, moved_future.get());
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, original_future.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    original_future.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_future_initialize_move_rebuilds_mutex,
        "ft_future initialize(move) reinstates mutex and preserves destination usability")
{
    ft_promise<int> source_promise;
    ft_future<int> source_future(source_promise);
    ft_promise<int> destination_promise;
    ft_future<int> destination_future(destination_promise);
    bool destination_lock_acquired;
    bool source_lock_acquired;

    source_promise.set_value(47);
    destination_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_future.initialize(ft_move(source_future)));
    FT_ASSERT(destination_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, destination_future.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    destination_future.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(47, destination_future.get());
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());
    FT_ASSERT(source_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, source_future.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_future.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_future_void_move_rebuilds_mutex,
        "ft_future<void> move semantics recreate mutex and keep moved-from reusable")
{
    ft_promise<void> promise;
    ft_future<void> original_future(promise);
    ft_future<void> moved_future;
    bool moved_lock_acquired;

    moved_lock_acquired = false;
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(original_future)));
    FT_ASSERT(moved_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, moved_future.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    moved_future.unlock(moved_lock_acquired);
    promise.set_value();
    moved_future.get();
    FT_ASSERT_EQ(0, original_future.enable_thread_safety());
    FT_ASSERT(original_future.is_thread_safe_enabled());
    return (1);
}
