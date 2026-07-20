#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_currency_rate_additional_get_rate_to_base(game_currency_rate &value)
{
    (void)value.get_rate_to_base();
    return ;
}

static void
game_currency_rate_additional_disable_thread_safety(game_currency_rate &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_currency_rate_additional_unlock(game_currency_rate &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void
game_currency_rate_additional_initialize_twice(game_currency_rate &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_currency_rate_additional_copy_initialize_uninitialised(
    game_currency_rate &value)
{
    game_currency_rate source;

    (void)value.initialize(source);
    return ;
}

static void game_currency_rate_additional_move_initialize_uninitialised(
    game_currency_rate &value)
{
    game_currency_rate source;

    (void)value.initialize(static_cast<game_currency_rate &&>(source));
    return ;
}

static void
game_currency_rate_additional_move_uninitialised(game_currency_rate &value)
{
    game_currency_rate source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_currency_rate_get_rate_to_base_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_additional_get_rate_to_base));
    return (1);
}

FT_TEST(test_game_currency_rate_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_additional_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_currency_rate_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_additional_unlock));
    return (1);
}

FT_TEST(test_game_currency_rate_is_thread_safe_uninitialised_is_safe)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_currency_rate_destroy_uninitialised_is_safe)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_currency_rate_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_currency_rate_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_currency_rate>(
               game_currency_rate_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_currency_rate_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_currency_rate>(
               game_currency_rate_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_currency_rate_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_currency_rate>(
                        game_currency_rate_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_currency_rate_initialize_with_values_succeeds)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(7, 2.5, 4));
    FT_ASSERT_EQ(7, value.get_currency_id());
    FT_ASSERT_EQ(2.5, value.get_rate_to_base());
    FT_ASSERT_EQ(4, value.get_display_precision());
    return (1);
}

FT_TEST(test_game_currency_rate_initialize_default_values_succeeds)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_currency_id());
    FT_ASSERT_EQ(1.0, value.get_rate_to_base());
    FT_ASSERT_EQ(2, value.get_display_precision());
    return (1);
}

FT_TEST(test_game_currency_rate_get_currency_id_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(11, 3.0, 6));
    FT_ASSERT_EQ(11, value.get_currency_id());
    return (1);
}

FT_TEST(test_game_currency_rate_get_rate_to_base_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(11, 3.0, 6));
    FT_ASSERT_EQ(3.0, value.get_rate_to_base());
    return (1);
}

FT_TEST(test_game_currency_rate_get_display_precision_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(11, 3.0, 6));
    FT_ASSERT_EQ(6, value.get_display_precision());
    return (1);
}

FT_TEST(test_game_currency_rate_set_currency_id_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_currency_id(9);
    FT_ASSERT_EQ(9, value.get_currency_id());
    return (1);
}

FT_TEST(test_game_currency_rate_set_rate_to_base_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_rate_to_base(4.5);
    FT_ASSERT_EQ(4.5, value.get_rate_to_base());
    return (1);
}

FT_TEST(test_game_currency_rate_set_display_precision_initialised)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_display_precision(8);
    FT_ASSERT_EQ(8, value.get_display_precision());
    return (1);
}

FT_TEST(test_game_currency_rate_thread_safety_enable_disable_cycle)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_currency_rate_lock_unlock_initialised)
{
    game_currency_rate value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_currency_rate_destroy_twice_is_safe)
{
    game_currency_rate value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
