#include "../../Modules/Game/game_resistance.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_resistance_set_values(game_resistance &value)
{
    (void)value.set_values(10, 2);
    return ;
}

static void game_resistance_set_percent(game_resistance &value)
{
    (void)value.set_percent(10);
    return ;
}

static void game_resistance_set_flat(game_resistance &value)
{
    (void)value.set_flat(2);
    return ;
}

static void game_resistance_get_percent(game_resistance &value)
{
    (void)value.get_percent();
    return ;
}

static void game_resistance_get_flat(game_resistance &value)
{
    (void)value.get_flat();
    return ;
}

static void game_resistance_reset(game_resistance &value)
{
    (void)value.reset();
    return ;
}

static void game_resistance_get_error(game_resistance &value)
{
    (void)value.get_error();
    return ;
}

static void game_resistance_get_error_str(game_resistance &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_resistance_enable_thread_safety(game_resistance &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_resistance_disable_thread_safety(game_resistance &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_resistance_is_thread_safe(game_resistance &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_resistance_set_values_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_set_values));
    return (1);
}

FT_TEST(test_game_resistance_set_percent_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_set_percent));
    return (1);
}

FT_TEST(test_game_resistance_set_flat_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_set_flat));
    return (1);
}

FT_TEST(test_game_resistance_get_percent_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_get_percent));
    return (1);
}

FT_TEST(test_game_resistance_get_flat_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_get_flat));
    return (1);
}

FT_TEST(test_game_resistance_reset_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_reset));
    return (1);
}

FT_TEST(test_game_resistance_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_get_error));
    return (1);
}

FT_TEST(test_game_resistance_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_get_error_str));
    return (1);
}

FT_TEST(test_game_resistance_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_resistance_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_resistance_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_resistance>(
        game_resistance_is_thread_safe));
    return (1);
}
