#include "../../Template/future.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_future_thread_safety_controls,
        "ft_future installs optional mutex guards and preserves errno")
{
    ft_promise<int> promise;
    ft_future<int> future(promise);
    bool lock_acquired;

    FT_ASSERT_EQ(false, future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, future.enable_thread_safety());
    FT_ASSERT_EQ(true, future.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, future.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    future.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    future.disable_thread_safety();
    FT_ASSERT_EQ(false, future.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_future_move_transfers_thread_safety,
        "ft_future move operations transfer thread safety state")
{
    ft_promise<int> base_promise;
    ft_future<int> original(base_promise);

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());

    ft_future<int> moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original.is_thread_safe_enabled());

    ft_promise<int> extra_promise;
    ft_future<int> assigned(extra_promise);
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_future_void_thread_safety_controls,
        "ft_future<void> exposes the same guard management helpers")
{
    ft_promise<void> promise;
    ft_future<void> future(promise);
    bool lock_acquired;

    FT_ASSERT_EQ(false, future.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, future.enable_thread_safety());
    FT_ASSERT_EQ(true, future.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_STATE;
    lock_acquired = false;
    FT_ASSERT_EQ(0, future.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, ft_errno);
    future.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, ft_errno);
    future.disable_thread_safety();
    FT_ASSERT_EQ(false, future.is_thread_safe_enabled());
    return (1);
}
