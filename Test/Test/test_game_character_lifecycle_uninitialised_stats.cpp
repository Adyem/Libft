#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_get_hit_points(game_character &value)
{
    (void)value.get_hit_points();
    return ;
}

static void game_character_set_hit_points(game_character &value)
{
    value.set_hit_points(1);
    return ;
}

static void game_character_is_alive(game_character &value)
{
    (void)value.is_alive();
    return ;
}

static void game_character_get_physical_armor(game_character &value)
{
    (void)value.get_physical_armor();
    return ;
}

static void game_character_set_physical_armor(game_character &value)
{
    value.set_physical_armor(1);
    return ;
}

static void game_character_get_magic_armor(game_character &value)
{
    (void)value.get_magic_armor();
    return ;
}

static void game_character_set_magic_armor(game_character &value)
{
    value.set_magic_armor(1);
    return ;
}

static void game_character_get_current_physical_armor(game_character &value)
{
    (void)value.get_current_physical_armor();
    return ;
}

static void game_character_set_current_physical_armor(game_character &value)
{
    value.set_current_physical_armor(1);
    return ;
}

static void game_character_get_current_magic_armor(game_character &value)
{
    (void)value.get_current_magic_armor();
    return ;
}

static void game_character_set_current_magic_armor(game_character &value)
{
    value.set_current_magic_armor(1);
    return ;
}

static void game_character_restore_physical_armor(game_character &value)
{
    value.restore_physical_armor();
    return ;
}

static void game_character_restore_magic_armor(game_character &value)
{
    value.restore_magic_armor();
    return ;
}

static void game_character_restore_armor(game_character &value)
{
    value.restore_armor();
    return ;
}

static void game_character_set_damage_rule(game_character &value)
{
    value.set_damage_rule(FT_DAMAGE_RULE_FLAT);
    return ;
}

static void game_character_get_damage_rule(game_character &value)
{
    (void)value.get_damage_rule();
    return ;
}

static void game_character_get_might(game_character &value)
{
    (void)value.get_might();
    return ;
}

static void game_character_set_might(game_character &value)
{
    value.set_might(1);
    return ;
}

static void game_character_get_agility(game_character &value)
{
    (void)value.get_agility();
    return ;
}

static void game_character_set_agility(game_character &value)
{
    value.set_agility(1);
    return ;
}

FT_TEST(test_game_character_get_hit_points_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_hit_points));
    return (1);
}

FT_TEST(test_game_character_set_hit_points_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_hit_points));
    return (1);
}

FT_TEST(test_game_character_is_alive_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_is_alive));
    return (1);
}

FT_TEST(test_game_character_get_physical_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_physical_armor));
    return (1);
}

FT_TEST(test_game_character_set_physical_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_physical_armor));
    return (1);
}

FT_TEST(test_game_character_get_magic_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_magic_armor));
    return (1);
}

FT_TEST(test_game_character_set_magic_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_magic_armor));
    return (1);
}

FT_TEST(test_game_character_get_current_physical_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_current_physical_armor));
    return (1);
}

FT_TEST(test_game_character_set_current_physical_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_current_physical_armor));
    return (1);
}

FT_TEST(test_game_character_get_current_magic_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_current_magic_armor));
    return (1);
}

FT_TEST(test_game_character_set_current_magic_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_current_magic_armor));
    return (1);
}

FT_TEST(test_game_character_restore_physical_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_restore_physical_armor));
    return (1);
}

FT_TEST(test_game_character_restore_magic_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_restore_magic_armor));
    return (1);
}

FT_TEST(test_game_character_restore_armor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_restore_armor));
    return (1);
}

FT_TEST(test_game_character_set_damage_rule_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_damage_rule));
    return (1);
}

FT_TEST(test_game_character_get_damage_rule_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_damage_rule));
    return (1);
}

FT_TEST(test_game_character_get_might_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_might));
    return (1);
}

FT_TEST(test_game_character_set_might_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_might));
    return (1);
}

FT_TEST(test_game_character_get_agility_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_agility));
    return (1);
}

FT_TEST(test_game_character_set_agility_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_agility));
    return (1);
}
