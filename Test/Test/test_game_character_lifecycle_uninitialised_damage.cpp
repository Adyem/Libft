#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_take_damage(game_character &value)
{
    value.take_damage(1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void game_character_take_damage_flat(game_character &value)
{
    value.take_damage_flat(1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void game_character_take_damage_scaled(game_character &value)
{
    value.take_damage_scaled(1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void game_character_take_damage_buffer(game_character &value)
{
    value.take_damage_buffer(1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void game_character_take_damage_magic_shield(game_character &value)
{
    value.take_damage_magic_shield(1, FT_DAMAGE_PHYSICAL);
    return ;
}

FT_TEST(test_game_character_take_damage_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_take_damage));
    return (1);
}

FT_TEST(test_game_character_take_damage_flat_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_take_damage_flat));
    return (1);
}

FT_TEST(test_game_character_take_damage_scaled_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_take_damage_scaled));
    return (1);
}

FT_TEST(test_game_character_take_damage_buffer_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_take_damage_buffer));
    return (1);
}

FT_TEST(test_game_character_take_damage_magic_shield_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_take_damage_magic_shield));
    return (1);
}
