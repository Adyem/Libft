#include "../../Template/variant.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_variant_thread_safety_resets_errno,
        "ft_variant installs optional mutex guards and resets errno to success")
{
    ft_variant<int, const char*> variant_value;
    bool lock_acquired;

    FT_ASSERT_EQ(false, variant_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, variant_value.enable_thread_safety());
    FT_ASSERT_EQ(true, variant_value.is_thread_safe_enabled());
    variant_value.emplace<int>(42);
    ft_errno = FT_ERR_INVALID_OPERATION;
    lock_acquired = false;
    FT_ASSERT_EQ(0, variant_value.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    variant_value.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    variant_value.disable_thread_safety();
    FT_ASSERT_EQ(false, variant_value.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_variant_move_preserves_thread_safety,
        "ft_variant move operations transfer thread safety state")
{
    ft_variant<int, const char*> original;

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());
    original.emplace<int>(7);

    ft_variant<int, const char*> moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original.is_thread_safe_enabled());
    FT_ASSERT_EQ(7, moved.get<int>());

    ft_variant<int, const char*> assigned;
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    assigned.emplace<int>(3);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(7, assigned.get<int>());

    return (1);
}
