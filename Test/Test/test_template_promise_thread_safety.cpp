#include "../../Template/promise.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_promise_thread_safety_controls,
        "ft_promise optional guard can lock, unlock, and preserve errno")
{
    ft_promise<int> promise_value;
    bool lock_acquired;
    int saved_errno;

    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.disable_thread_safety();
    FT_ASSERT_EQ(false, promise_value.is_thread_safe());
    FT_ASSERT_EQ(0, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.set_value(42);
    saved_errno = FT_ERR_INVALID_OPERATION;
    ft_errno = saved_errno;
    lock_acquired = false;
    FT_ASSERT_EQ(0, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    promise_value.unlock(lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    return (1);
}

FT_TEST(test_promise_void_thread_safety_controls,
        "ft_promise<void> guard locks and unlocks without disturbing errno")
{
    ft_promise<void> promise_value;
    bool lock_acquired;
    int saved_errno;

    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.disable_thread_safety();
    FT_ASSERT_EQ(false, promise_value.is_thread_safe());
    FT_ASSERT_EQ(0, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.set_value();
    saved_errno = FT_ERR_INVALID_OPERATION;
    ft_errno = saved_errno;
    lock_acquired = false;
    FT_ASSERT_EQ(0, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    promise_value.unlock(lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    return (1);
}
