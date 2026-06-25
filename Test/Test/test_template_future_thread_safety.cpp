#include "../test_internal.hpp"
#include "../../Modules/Template/future.hpp"
#include "../../Modules/Template/promise.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_future_thread_safety_resets_errno)
{
    ft_promise<int> promise;
    ft_future<int> future(promise);
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, future.initialize());
    FT_ASSERT_EQ(false, future.is_thread_safe());
    FT_ASSERT_EQ(0, future.enable_thread_safety());
    FT_ASSERT_EQ(true, future.is_thread_safe());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, future.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    future.unlock(lock_acquired);
    future.disable_thread_safety();
    FT_ASSERT_EQ(false, future.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future.destroy());
    return (1);
}

FT_TEST(test_future_move_transfers_thread_safety)
{
    ft_promise<int> base_promise;
    ft_future<int> original(base_promise);
    ft_future<int> moved;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe());

    FT_ASSERT_EQ(0, moved.move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe());

    ft_promise<int> extra_promise;
    ft_future<int> assigned(extra_promise);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.initialize());
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    FT_ASSERT_EQ(0, assigned.move(moved));
    FT_ASSERT_EQ(true, assigned.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.destroy());
    return (1);
}

FT_TEST(test_future_move_constructor_transfers_thread_safety)
{
    ft_promise<int> promise;
    ft_future<int> original(promise);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe());

    ft_future<int> moved;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.move(original));

    FT_ASSERT_EQ(false, original.valid());
    FT_ASSERT_EQ(true, moved.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.destroy());
    return (1);
}

FT_TEST(test_future_void_thread_safety_controls)
{
    ft_promise<void> promise;
    ft_future<void> future(promise);
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, future.initialize());
    FT_ASSERT_EQ(false, future.is_thread_safe());
    FT_ASSERT_EQ(0, future.enable_thread_safety());
    FT_ASSERT_EQ(true, future.is_thread_safe());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, future.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    future.unlock(lock_acquired);
    future.disable_thread_safety();
    FT_ASSERT_EQ(false, future.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future.destroy());
    return (1);
}
