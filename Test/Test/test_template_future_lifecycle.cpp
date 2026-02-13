#include "../test_internal.hpp"
#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_future_default_is_invalid,
    "ft_future default constructor creates invalid future")
{
    ft_future<int> future_value;

    FT_ASSERT_EQ(false, future_value.valid());
    return (1);
}

FT_TEST(test_ft_future_initialize_copy_lifecycle,
    "ft_future initialize(copy) clones handle lifecycle")
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(101);
    FT_ASSERT_EQ(0, destination_future.initialize(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(101, destination_future.get());
    return (1);
}

FT_TEST(test_ft_future_initialize_move_lifecycle,
    "ft_future initialize(move) transfers lifecycle and invalidates source")
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(202);
    FT_ASSERT_EQ(0, destination_future.initialize(ft_move(source_future)));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(202, destination_future.get());
    return (1);
}

FT_TEST(test_ft_future_move_method_lifecycle,
    "ft_future move method transfers lifecycle to destination")
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(303);
    FT_ASSERT_EQ(0, destination_future.move(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(303, destination_future.get());
    return (1);
}

FT_TEST(test_ft_future_move_self_is_noop,
    "ft_future move(self) is a no-op success")
{
    ft_promise<int> promise_value;
    ft_future<int> future_value(promise_value);

    promise_value.set_value(404);
    FT_ASSERT_EQ(0, future_value.move(future_value));
    FT_ASSERT_EQ(true, future_value.valid());
    FT_ASSERT_EQ(404, future_value.get());
    return (1);
}

FT_TEST(test_ft_future_wait_and_error_on_invalid,
    "ft_future wait/get on invalid state keeps lifecycle stable")
{
    ft_future<int> future_value;
    int result_value;

    future_value.wait();
    result_value = future_value.get();
    FT_ASSERT_EQ(0, result_value);
    FT_ASSERT_EQ(false, future_value.valid());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, future_value.last_error_code());
    return (1);
}

FT_TEST(test_ft_future_thread_safety_member_lifecycle,
    "ft_future thread safety members work through lifecycle")
{
    ft_promise<int> promise_value;
    ft_future<int> future_value(promise_value);
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(false, future_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, future_value.enable_thread_safety());
    FT_ASSERT_EQ(true, future_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, future_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    future_value.unlock(lock_acquired);
    future_value.disable_thread_safety();
    FT_ASSERT_EQ(false, future_value.is_thread_safe_enabled());
    return (1);
}

#ifdef LIBFT_TEST_BUILD
FT_TEST(test_ft_future_validation_mutex_helper,
    "ft_future validation mutex helper exposes recursive mutex")
{
    ft_promise<int> promise_value;
    ft_future<int> future_value(promise_value);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = future_value.get_mutex_for_validation();
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, future_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, mutex_pointer->lock());
    FT_ASSERT_EQ(0, mutex_pointer->unlock());
    return (1);
}
#endif

FT_TEST(test_ft_future_void_initialize_and_get,
    "ft_future<void> initialize(copy/move) and get follow lifecycle")
{
    ft_promise<void> promise_value;
    ft_future<void> source_future(promise_value);
    ft_future<void> copy_future;
    ft_future<void> moved_future;

    FT_ASSERT_EQ(0, copy_future.initialize(source_future));
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(source_future)));
    FT_ASSERT_EQ(true, copy_future.valid());
    FT_ASSERT_EQ(true, moved_future.valid());
    promise_value.set_value();
    copy_future.get();
    moved_future.get();
    return (1);
}

FT_TEST(test_ft_future_void_move_method_and_thread_safety,
    "ft_future<void> move and thread safety members are lifecycle-safe")
{
    ft_promise<void> promise_value;
    ft_future<void> source_future(promise_value);
    ft_future<void> destination_future;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(0, destination_future.move(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(true, destination_future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, destination_future.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    destination_future.unlock(lock_acquired);
    destination_future.disable_thread_safety();
    FT_ASSERT_EQ(false, destination_future.is_thread_safe_enabled());
    return (1);
}

