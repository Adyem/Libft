#include "../test_internal.hpp"
#include "../../Template/promise.hpp"
#include "../../System_utils/test_runner.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_promise_thread_safety_controls_reset_errno,
        "ft_promise optional guard can lock, unlock, and reset errno")
{
    ft_promise<int> promise_value;
    bool lock_acquired;

    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.disable_thread_safety();
    FT_ASSERT_EQ(false, promise_value.is_thread_safe());
    FT_ASSERT_EQ(0, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.set_value(42);
    lock_acquired = false;
    FT_ASSERT_EQ(0, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    promise_value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_promise_void_thread_safety_controls_reset_errno,
        "ft_promise<void> guard locks and unlocks while resetting errno")
{
    ft_promise<void> promise_value;
    bool lock_acquired;

    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.disable_thread_safety();
    FT_ASSERT_EQ(false, promise_value.is_thread_safe());
    FT_ASSERT_EQ(0, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(true, promise_value.is_thread_safe());
    promise_value.set_value();
    lock_acquired = false;
    FT_ASSERT_EQ(0, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    promise_value.unlock(lock_acquired);
    return (1);
}
