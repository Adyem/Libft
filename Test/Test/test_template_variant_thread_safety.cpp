#include "../test_internal.hpp"
#include "../../Template/variant.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_variant_thread_safety_controls,
        "ft_variant optional guard locks and unlocks")
{
    ft_variant<int, const char*> variant_value;
    bool lock_acquired = false;

    FT_ASSERT_EQ(false, variant_value.is_thread_safe());
    FT_ASSERT_EQ(0, variant_value.enable_thread_safety());
    FT_ASSERT_EQ(true, variant_value.is_thread_safe());
    FT_ASSERT_EQ(0, variant_value.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    variant_value.unlock(lock_acquired);
    FT_ASSERT_EQ(0, variant_value.disable_thread_safety());
    FT_ASSERT_EQ(false, variant_value.is_thread_safe());
    return (1);
}

FT_TEST(test_variant_move_not_supported,
        "ft_variant move constructors/operators are deleted")
{
    FT_ASSERT((std::is_move_constructible<ft_variant<int, const char*>>::value == false));
    FT_ASSERT((std::is_move_assignable<ft_variant<int, const char*>>::value == false));
    return (1);
}
