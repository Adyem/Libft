#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void character_destroyed_get_hit_points(game_character &value)
{
    (void)value.get_hit_points();
    return ;
}

static void character_destroyed_set_hit_points(game_character &value)
{
    value.set_hit_points(1);
    return ;
}

static void character_destroyed_get_physical_armor(game_character &value)
{
    (void)value.get_physical_armor();
    return ;
}

static void character_destroyed_set_physical_armor(game_character &value)
{
    value.set_physical_armor(1);
    return ;
}

static void character_destroyed_restore_armor(game_character &value)
{
    value.restore_armor();
    return ;
}

static void character_destroyed_take_damage(game_character &value)
{
    value.take_damage(1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void character_destroyed_get_might(game_character &value)
{
    (void)value.get_might();
    return ;
}

static void character_destroyed_set_might(game_character &value)
{
    value.set_might(1);
    return ;
}

static void character_destroyed_get_coins(game_character &value)
{
    (void)value.get_coins();
    return ;
}

static void character_destroyed_set_coins(game_character &value)
{
    value.set_coins(1);
    return ;
}

FT_TEST(test_game_character_destroyed_get_hit_points_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_get_hit_points));
    return (1);
}

FT_TEST(test_game_character_destroyed_set_hit_points_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_set_hit_points));
    return (1);
}

FT_TEST(test_game_character_destroyed_get_physical_armor_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_get_physical_armor));
    return (1);
}

FT_TEST(test_game_character_destroyed_set_physical_armor_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_set_physical_armor));
    return (1);
}

FT_TEST(test_game_character_destroyed_restore_armor_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_restore_armor));
    return (1);
}

FT_TEST(test_game_character_destroyed_take_damage_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_take_damage));
    return (1);
}

FT_TEST(test_game_character_destroyed_get_might_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_get_might));
    return (1);
}

FT_TEST(test_game_character_destroyed_set_might_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_set_might));
    return (1);
}

FT_TEST(test_game_character_destroyed_get_coins_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_get_coins));
    return (1);
}

FT_TEST(test_game_character_destroyed_set_coins_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_character>(
                        character_destroyed_set_coins));
    return (1);
}
