#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_currency_rate_get_currency_id(game_currency_rate &value)
{
    (void)value.get_currency_id();
    return ;
}
static void game_currency_rate_set_currency_id(game_currency_rate &value)
{
    value.set_currency_id(1);
    return ;
}
static void game_currency_rate_set_rate_to_base(game_currency_rate &value)
{
    value.set_rate_to_base(1.0);
    return ;
}
static void game_currency_rate_get_display_precision(game_currency_rate &value)
{
    (void)value.get_display_precision();
    return ;
}
static void game_currency_rate_set_display_precision(game_currency_rate &value)
{
    value.set_display_precision(2);
    return ;
}
static void game_currency_rate_get_error(game_currency_rate &value)
{
    (void)value.get_error();
    return ;
}
static void game_currency_rate_get_error_str(game_currency_rate &value)
{
    (void)value.get_error_str();
    return ;
}
static void game_currency_rate_enable_thread_safety(game_currency_rate &value)
{
    (void)value.enable_thread_safety();
    return ;
}
static void game_currency_rate_lock(game_currency_rate &value)
{
    ft_bool lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}
static void game_currency_rate_move_self(game_currency_rate &value)
{
    (void)value.move(value);
    return ;
}

FT_TEST(test_new_game_currency_rate_get_currency_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_get_currency_id));
    return (1);
}
FT_TEST(test_new_game_currency_rate_set_currency_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_set_currency_id));
    return (1);
}
FT_TEST(test_new_game_currency_rate_set_rate_to_base_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_set_rate_to_base));
    return (1);
}
FT_TEST(test_new_game_currency_rate_get_display_precision_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_get_display_precision));
    return (1);
}
FT_TEST(test_new_game_currency_rate_set_display_precision_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_set_display_precision));
    return (1);
}
FT_TEST(test_new_game_currency_rate_get_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_get_error));
    return (1);
}
FT_TEST(test_new_game_currency_rate_get_error_str_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_get_error_str));
    return (1);
}
FT_TEST(test_new_game_currency_rate_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_enable_thread_safety));
    return (1);
}
FT_TEST(test_new_game_currency_rate_lock_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_lock));
    return (1);
}
FT_TEST(test_new_game_currency_rate_move_self_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_move_self));
    return (1);
}
