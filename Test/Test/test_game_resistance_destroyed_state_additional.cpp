#include "../../Modules/Game/game_resistance.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void resistance_destroyed_set_values(game_resistance &value)
{
    (void)value.set_values(1, 1);
    return ;
}

static void resistance_destroyed_set_percent(game_resistance &value)
{
    (void)value.set_percent(1);
    return ;
}

static void resistance_destroyed_set_flat(game_resistance &value)
{
    (void)value.set_flat(1);
    return ;
}

static void resistance_destroyed_get_percent(game_resistance &value)
{
    (void)value.get_percent();
    return ;
}

static void resistance_destroyed_get_flat(game_resistance &value)
{
    (void)value.get_flat();
    return ;
}

static void resistance_destroyed_reset(game_resistance &value)
{
    (void)value.reset();
    return ;
}

static void resistance_destroyed_enable_thread_safety(game_resistance &value)
{
    (void)value.enable_thread_safety();
    return ;
}

FT_TEST(test_game_resistance_destroyed_set_values_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_set_values));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_set_percent_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_set_percent));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_set_flat_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_set_flat));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_get_percent_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_get_percent));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_get_flat_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_get_flat));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_reset_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_reset));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_resistance>(
                        resistance_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_resistance_destroyed_get_error_aborts)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_resistance_destroyed_get_error_str_is_valid)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_resistance_destroyed_disable_thread_safety_is_safe)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}
