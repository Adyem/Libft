#include "../test_internal.hpp"
#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_future_default_is_invalid)
{
    ft_future<int> future_value;

    FT_ASSERT_EQ(false, future_value.valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
    return (1);
}

FT_TEST(test_ft_future_initialize_copy_lifecycle)
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(101);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(0, destination_future.initialize(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(101, destination_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_initialize_move_lifecycle)
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(202);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(0, destination_future.initialize(ft_move(source_future)));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(202, destination_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_move_method_lifecycle)
{
    ft_promise<int> promise_value;
    ft_future<int> source_future(promise_value);
    ft_future<int> destination_future;

    promise_value.set_value(303);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(0, destination_future.move(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(303, destination_future.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_move_self_is_noop)
{
    ft_promise<int> promise_value;
    ft_future<int> future_value(promise_value);

    promise_value.set_value(404);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.initialize());
    FT_ASSERT_EQ(0, future_value.move(future_value));
    FT_ASSERT_EQ(true, future_value.valid());
    FT_ASSERT_EQ(404, future_value.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
    return (1);
}

FT_TEST(test_ft_future_wait_and_error_on_invalid)
{
    ft_future<int> future_value;
    int result_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.initialize());
    future_value.wait();
    result_value = future_value.get();
    FT_ASSERT_EQ(0, result_value);
    FT_ASSERT_EQ(false, future_value.valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
    return (1);
}

FT_TEST(test_ft_future_thread_safety_member_lifecycle)
{
    ft_promise<int> promise_value;
    ft_future<int> future_value(promise_value);
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.initialize());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(false, future_value.is_thread_safe());
    FT_ASSERT_EQ(0, future_value.enable_thread_safety());
    FT_ASSERT_EQ(true, future_value.is_thread_safe());
    FT_ASSERT_EQ(0, future_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    future_value.unlock(lock_acquired);
    future_value.disable_thread_safety();
    FT_ASSERT_EQ(false, future_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
    return (1);
}

FT_TEST(test_ft_future_void_initialize_and_get)
{
    ft_promise<void> promise_value;
    ft_future<void> source_future(promise_value);
    ft_future<void> copy_future;
    ft_future<void> moved_future;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(0, copy_future.initialize(source_future));
    FT_ASSERT_EQ(0, moved_future.initialize(ft_move(source_future)));
    FT_ASSERT_EQ(true, copy_future.valid());
    FT_ASSERT_EQ(true, moved_future.valid());
    promise_value.set_value();
    copy_future.get();
    moved_future.get();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_void_move_method_and_thread_safety)
{
    ft_promise<void> promise_value;
    ft_future<void> source_future(promise_value);
    ft_future<void> destination_future;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, destination_future.move(source_future));
    FT_ASSERT_EQ(true, destination_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(0, destination_future.enable_thread_safety());
    FT_ASSERT_EQ(true, destination_future.is_thread_safe());
    FT_ASSERT_EQ(0, destination_future.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    destination_future.unlock(lock_acquired);
    destination_future.disable_thread_safety();
    FT_ASSERT_EQ(false, destination_future.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}

FT_TEST(test_ft_future_void_move_constructor_and_thread_safety)
{
    ft_promise<void> promise_value;
    ft_future<void> source_future(promise_value);
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.initialize());
    FT_ASSERT_EQ(0, source_future.enable_thread_safety());

    ft_future<void> moved_future(static_cast<ft_future<void> &&>(source_future));

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(true, moved_future.valid());
    FT_ASSERT_EQ(false, source_future.valid());
    FT_ASSERT_EQ(true, moved_future.is_thread_safe());
    FT_ASSERT_EQ(0, moved_future.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    moved_future.unlock(lock_acquired);
    promise_value.set_value();
    moved_future.get();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, moved_future.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_future.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_future.destroy());
    return (1);
}
