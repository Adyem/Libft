#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_skill_get_id(game_skill &value)
{
    (void)value.get_id();
    return ;
}
static void game_skill_set_id(game_skill &value)
{
    value.set_id(1);
    return ;
}
static void game_skill_get_level(game_skill &value)
{
    (void)value.get_level();
    return ;
}
static void game_skill_set_level(game_skill &value)
{
    value.set_level(1);
    return ;
}
static void game_skill_get_cooldown(game_skill &value)
{
    (void)value.get_cooldown();
    return ;
}
static void game_skill_set_cooldown(game_skill &value)
{
    value.set_cooldown(1);
    return ;
}
static void game_skill_add_cooldown(game_skill &value)
{
    value.add_cooldown(1);
    return ;
}
static void game_skill_sub_cooldown(game_skill &value)
{
    value.sub_cooldown(1);
    return ;
}
static void game_skill_get_modifier1(game_skill &value)
{
    (void)value.get_modifier1();
    return ;
}
static void game_skill_set_modifier1(game_skill &value)
{
    value.set_modifier1(1);
    return ;
}

FT_TEST(test_new_game_skill_get_id_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_skill>(game_skill_get_id));
    return (1);
}
FT_TEST(test_new_game_skill_set_id_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_skill>(game_skill_set_id));
    return (1);
}
FT_TEST(test_new_game_skill_get_level_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_get_level));
    return (1);
}
FT_TEST(test_new_game_skill_set_level_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_set_level));
    return (1);
}
FT_TEST(test_new_game_skill_get_cooldown_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_get_cooldown));
    return (1);
}
FT_TEST(test_new_game_skill_set_cooldown_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_set_cooldown));
    return (1);
}
FT_TEST(test_new_game_skill_add_cooldown_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_add_cooldown));
    return (1);
}
FT_TEST(test_new_game_skill_sub_cooldown_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_sub_cooldown));
    return (1);
}
FT_TEST(test_new_game_skill_get_modifier1_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_get_modifier1));
    return (1);
}
FT_TEST(test_new_game_skill_set_modifier1_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_skill>(game_skill_set_modifier1));
    return (1);
}
