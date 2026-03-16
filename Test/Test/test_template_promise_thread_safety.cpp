#include "../test_internal.hpp"
#include "../../Template/promise.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_promise_thread_safety_controls)
{
    ft_promise<int32_t> promise_value;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_FALSE, promise_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, promise_value.is_thread_safe());
    promise_value.set_value(42);
    FT_ASSERT_EQ(FT_TRUE, promise_value.is_ready());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    promise_value.unlock(lock_acquired);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, promise_value.is_thread_safe());
    return (1);
}

FT_TEST(test_template_promise_void_thread_safety_controls)
{
    ft_promise<void> promise_value;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_FALSE, promise_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, promise_value.is_thread_safe());
    promise_value.set_value();
    FT_ASSERT_EQ(FT_TRUE, promise_value.is_ready());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    promise_value.unlock(lock_acquired);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, promise_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, promise_value.is_thread_safe());
    return (1);
}
