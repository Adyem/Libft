#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_rarity_band_get_rarity(game_rarity_band &value)
{
    (void)value.get_rarity();
    return ;
}

static void game_rarity_band_set_rarity(game_rarity_band &value)
{
    value.set_rarity(1);
    return ;
}

static void game_rarity_band_get_value_multiplier(game_rarity_band &value)
{
    (void)value.get_value_multiplier();
    return ;
}

static void game_rarity_band_set_value_multiplier(game_rarity_band &value)
{
    value.set_value_multiplier(1.0);
    return ;
}

static void game_rarity_band_enable_thread_safety(game_rarity_band &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_rarity_band_disable_thread_safety(game_rarity_band &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_rarity_band_is_thread_safe(game_rarity_band &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_rarity_band_get_error(game_rarity_band &value)
{
    (void)value.get_error();
    return ;
}

static void game_rarity_band_get_error_str(game_rarity_band &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_rarity_band_get_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_get_rarity));
    return (1);
}

FT_TEST(test_game_rarity_band_set_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_set_rarity));
    return (1);
}

FT_TEST(test_game_rarity_band_get_value_multiplier_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_get_value_multiplier));
    return (1);
}

FT_TEST(test_game_rarity_band_set_value_multiplier_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_set_value_multiplier));
    return (1);
}

FT_TEST(test_game_rarity_band_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_rarity_band_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_rarity_band_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_is_thread_safe));
    return (1);
}

FT_TEST(test_game_rarity_band_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_get_error));
    return (1);
}

FT_TEST(test_game_rarity_band_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_get_error_str));
    return (1);
}
