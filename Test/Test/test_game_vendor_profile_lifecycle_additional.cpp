#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_vendor_profile_additional_get_buy_markup(game_vendor_profile &value)
{
    (void)value.get_buy_markup();
    return ;
}

static void
game_vendor_profile_additional_get_sell_multiplier(game_vendor_profile &value)
{
    (void)value.get_sell_multiplier();
    return ;
}

static void
game_vendor_profile_additional_get_tax_rate(game_vendor_profile &value)
{
    (void)value.get_tax_rate();
    return ;
}

static void
game_vendor_profile_additional_disable_thread_safety(game_vendor_profile &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_vendor_profile_additional_unlock(game_vendor_profile &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void
game_vendor_profile_additional_initialize_twice(game_vendor_profile &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_vendor_profile_additional_copy_initialize_uninitialised(
    game_vendor_profile &value)
{
    game_vendor_profile source;

    (void)value.initialize(source);
    return ;
}

static void game_vendor_profile_additional_move_initialize_uninitialised(
    game_vendor_profile &value)
{
    game_vendor_profile source;

    (void)value.initialize(static_cast<game_vendor_profile &&>(source));
    return ;
}

static void
game_vendor_profile_additional_move_uninitialised(game_vendor_profile &value)
{
    game_vendor_profile source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_vendor_profile_get_buy_markup_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_get_buy_markup));
    return (1);
}

FT_TEST(test_game_vendor_profile_get_sell_multiplier_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_get_sell_multiplier));
    return (1);
}

FT_TEST(test_game_vendor_profile_get_tax_rate_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_get_tax_rate));
    return (1);
}

FT_TEST(test_game_vendor_profile_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_vendor_profile_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_unlock));
    return (1);
}

FT_TEST(test_game_vendor_profile_is_thread_safe_uninitialised_is_safe)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_vendor_profile_destroy_uninitialised_is_safe)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_vendor_profile_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_vendor_profile_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
               game_vendor_profile_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_vendor_profile_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
               game_vendor_profile_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_vendor_profile_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_vendor_profile>(
                        game_vendor_profile_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_vendor_profile_initialize_with_values_succeeds)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(4, 1.25, 0.75, 0.08));
    FT_ASSERT_EQ(4, value.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.25, value.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.75, value.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.08, value.get_tax_rate());
    return (1);
}

FT_TEST(test_game_vendor_profile_initialize_default_values_succeeds)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.0, value.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(1.0, value.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.0, value.get_tax_rate());
    return (1);
}

FT_TEST(test_game_vendor_profile_get_vendor_id_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(12, 1.1, 0.9, 0.04));
    FT_ASSERT_EQ(12, value.get_vendor_id());
    return (1);
}

FT_TEST(test_game_vendor_profile_get_buy_markup_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(12, 1.1, 0.9, 0.04));
    FT_ASSERT_DOUBLE_EQ(1.1, value.get_buy_markup());
    return (1);
}

FT_TEST(test_game_vendor_profile_get_sell_multiplier_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(12, 1.1, 0.9, 0.04));
    FT_ASSERT_DOUBLE_EQ(0.9, value.get_sell_multiplier());
    return (1);
}

FT_TEST(test_game_vendor_profile_get_tax_rate_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(12, 1.1, 0.9, 0.04));
    FT_ASSERT_DOUBLE_EQ(0.04, value.get_tax_rate());
    return (1);
}

FT_TEST(test_game_vendor_profile_set_vendor_id_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_vendor_id(15);
    FT_ASSERT_EQ(15, value.get_vendor_id());
    return (1);
}

FT_TEST(test_game_vendor_profile_set_buy_markup_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_buy_markup(1.4);
    FT_ASSERT_DOUBLE_EQ(1.4, value.get_buy_markup());
    return (1);
}

FT_TEST(test_game_vendor_profile_set_sell_multiplier_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_sell_multiplier(0.6);
    FT_ASSERT_DOUBLE_EQ(0.6, value.get_sell_multiplier());
    return (1);
}

FT_TEST(test_game_vendor_profile_set_tax_rate_initialised)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_tax_rate(0.15);
    FT_ASSERT_DOUBLE_EQ(0.15, value.get_tax_rate());
    return (1);
}

FT_TEST(test_game_vendor_profile_thread_safety_enable_disable_cycle)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_vendor_profile_lock_unlock_initialised)
{
    game_vendor_profile value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_vendor_profile_destroy_twice_is_safe)
{
    game_vendor_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
