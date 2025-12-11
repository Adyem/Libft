#include "../../Template/function.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

static int increment_value(int value)
{
    return (value + 1);
}

FT_TEST(test_function_thread_safety_resets_errno,
        "ft_function toggles optional mutex guard and resets errno to success")
{
    ft_function<int(int)> function_value(increment_value);
    bool lock_acquired;

    FT_ASSERT_EQ(false, function_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, function_value.enable_thread_safety());
    FT_ASSERT_EQ(true, function_value.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_OPERATION;
    lock_acquired = false;
    FT_ASSERT_EQ(0, function_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    function_value.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(3, function_value(2));
    function_value.disable_thread_safety();
    FT_ASSERT_EQ(false, function_value.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_function_move_transfers_thread_safety,
        "ft_function move operations transfer thread safety state")
{
    ft_function<int(int)> original(increment_value);

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, original(4));

    ft_function<int(int)> moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original.is_thread_safe_enabled());
    FT_ASSERT_EQ(6, moved(5));

    ft_function<int(int)> assigned(increment_value);
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(7, assigned(6));

    return (1);
}
