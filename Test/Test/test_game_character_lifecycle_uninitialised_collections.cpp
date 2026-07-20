#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_get_skills(game_character &value)
{
    (void)value.get_skills();
    return ;
}

static void game_character_get_skills_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_skills();
    return ;
}

static void game_character_get_skill(game_character &value)
{
    (void)value.get_skill(1);
    return ;
}

static void game_character_get_skill_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_skill(1);
    return ;
}

static void game_character_add_skill(game_character &value)
{
    game_skill skill;

    (void)value.add_skill(skill);
    return ;
}

static void game_character_remove_skill(game_character &value)
{
    value.remove_skill(1);
    return ;
}

static void game_character_get_buffs(game_character &value)
{
    (void)value.get_buffs();
    return ;
}

static void game_character_get_buffs_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_buffs();
    return ;
}

static void game_character_get_debuffs(game_character &value)
{
    (void)value.get_debuffs();
    return ;
}

static void game_character_get_debuffs_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_debuffs();
    return ;
}

static void game_character_get_upgrades(game_character &value)
{
    (void)value.get_upgrades();
    return ;
}

static void game_character_get_upgrades_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_upgrades();
    return ;
}

static void game_character_get_quests(game_character &value)
{
    (void)value.get_quests();
    return ;
}

static void game_character_get_quests_const(game_character &value)
{
    const game_character &constant_value = value;

    (void)constant_value.get_quests();
    return ;
}

FT_TEST(test_game_character_get_skills_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_skills));
    return (1);
}

FT_TEST(test_game_character_get_skills_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_skills_const));
    return (1);
}

FT_TEST(test_game_character_get_skill_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_skill));
    return (1);
}

FT_TEST(test_game_character_get_skill_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_skill_const));
    return (1);
}

FT_TEST(test_game_character_add_skill_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_add_skill));
    return (1);
}

FT_TEST(test_game_character_remove_skill_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_remove_skill));
    return (1);
}

FT_TEST(test_game_character_get_buffs_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_buffs));
    return (1);
}

FT_TEST(test_game_character_get_buffs_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_buffs_const));
    return (1);
}

FT_TEST(test_game_character_get_debuffs_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_debuffs));
    return (1);
}

FT_TEST(test_game_character_get_debuffs_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_debuffs_const));
    return (1);
}

FT_TEST(test_game_character_get_upgrades_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_upgrades));
    return (1);
}

FT_TEST(test_game_character_get_upgrades_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_upgrades_const));
    return (1);
}

FT_TEST(test_game_character_get_quests_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_quests));
    return (1);
}

FT_TEST(test_game_character_get_quests_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, run_game_lifecycle_default<game_character>(
                        game_character_get_quests_const));
    return (1);
}
