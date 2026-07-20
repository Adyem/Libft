#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_vendor_profile_get_vendor_id(game_vendor_profile &value)
{
    (void)value.get_vendor_id();
    return ;
}
static void game_vendor_profile_set_vendor_id(game_vendor_profile &value)
{
    value.set_vendor_id(1);
    return ;
}
static void game_vendor_profile_set_buy_markup(game_vendor_profile &value)
{
    value.set_buy_markup(1.0);
    return ;
}
static void game_vendor_profile_set_sell_multiplier(game_vendor_profile &value)
{
    value.set_sell_multiplier(1.0);
    return ;
}
static void game_vendor_profile_set_tax_rate(game_vendor_profile &value)
{
    value.set_tax_rate(1.0);
    return ;
}
static void game_vendor_profile_get_error(game_vendor_profile &value)
{
    (void)value.get_error();
    return ;
}
static void game_vendor_profile_get_error_str(game_vendor_profile &value)
{
    (void)value.get_error_str();
    return ;
}
static void game_vendor_profile_enable_thread_safety(game_vendor_profile &value)
{
    (void)value.enable_thread_safety();
    return ;
}
static void game_vendor_profile_lock(game_vendor_profile &value)
{
    ft_bool lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}
static void game_vendor_profile_move_self(game_vendor_profile &value)
{
    (void)value.move(value);
    return ;
}

FT_TEST(test_new_game_vendor_profile_get_vendor_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_get_vendor_id));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_set_vendor_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_set_vendor_id));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_set_buy_markup_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_set_buy_markup));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_set_sell_multiplier_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_set_sell_multiplier));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_set_tax_rate_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_set_tax_rate));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_get_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_get_error));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_get_error_str_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_get_error_str));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_enable_thread_safety));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_lock_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_lock));
    return (1);
}
FT_TEST(test_new_game_vendor_profile_move_self_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_move_self));
    return (1);
}
