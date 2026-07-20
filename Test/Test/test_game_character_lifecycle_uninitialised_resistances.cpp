#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_get_fire_res(game_character &value)
{
    (void)value.get_fire_res();
    return ;
}

static void game_character_set_fire_res(game_character &value)
{
    value.set_fire_res(1, 1);
    return ;
}

static void game_character_get_frost_res(game_character &value)
{
    (void)value.get_frost_res();
    return ;
}

static void game_character_set_frost_res(game_character &value)
{
    value.set_frost_res(1, 1);
    return ;
}

static void game_character_get_lightning_res(game_character &value)
{
    (void)value.get_lightning_res();
    return ;
}

static void game_character_set_lightning_res(game_character &value)
{
    value.set_lightning_res(1, 1);
    return ;
}

static void game_character_get_air_res(game_character &value)
{
    (void)value.get_air_res();
    return ;
}

static void game_character_set_air_res(game_character &value)
{
    value.set_air_res(1, 1);
    return ;
}

static void game_character_get_earth_res(game_character &value)
{
    (void)value.get_earth_res();
    return ;
}

static void game_character_set_earth_res(game_character &value)
{
    value.set_earth_res(1, 1);
    return ;
}

static void game_character_get_chaos_res(game_character &value)
{
    (void)value.get_chaos_res();
    return ;
}

static void game_character_set_chaos_res(game_character &value)
{
    value.set_chaos_res(1, 1);
    return ;
}

static void game_character_get_physical_res(game_character &value)
{
    (void)value.get_physical_res();
    return ;
}

static void game_character_set_physical_res(game_character &value)
{
    value.set_physical_res(1, 1);
    return ;
}

FT_TEST(test_game_character_get_fire_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_fire_res));
    return (1);
}

FT_TEST(test_game_character_set_fire_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_fire_res));
    return (1);
}

FT_TEST(test_game_character_get_frost_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_frost_res));
    return (1);
}

FT_TEST(test_game_character_set_frost_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_frost_res));
    return (1);
}

FT_TEST(test_game_character_get_lightning_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_lightning_res));
    return (1);
}

FT_TEST(test_game_character_set_lightning_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_lightning_res));
    return (1);
}

FT_TEST(test_game_character_get_air_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_air_res));
    return (1);
}

FT_TEST(test_game_character_set_air_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_air_res));
    return (1);
}

FT_TEST(test_game_character_get_earth_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_earth_res));
    return (1);
}

FT_TEST(test_game_character_set_earth_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_earth_res));
    return (1);
}

FT_TEST(test_game_character_get_chaos_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_chaos_res));
    return (1);
}

FT_TEST(test_game_character_set_chaos_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_chaos_res));
    return (1);
}

FT_TEST(test_game_character_get_physical_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_physical_res));
    return (1);
}

FT_TEST(test_game_character_set_physical_res_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_physical_res));
    return (1);
}
