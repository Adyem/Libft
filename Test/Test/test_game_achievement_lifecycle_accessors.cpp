#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_achievement_get_id(game_achievement &value)
{
    (void)value.get_id();
    return ;
}
static void game_achievement_set_id(game_achievement &value)
{
    value.set_id(4);
    return ;
}
static void game_achievement_get_goals(game_achievement &value)
{
    (void)value.get_goals();
    return ;
}
static void game_achievement_get_goal(game_achievement &value)
{
    (void)value.get_goal(1);
    return ;
}
static void game_achievement_set_goal(game_achievement &value)
{
    value.set_goal(1, 2);
    return ;
}
static void game_achievement_get_progress(game_achievement &value)
{
    (void)value.get_progress(1);
    return ;
}
static void game_achievement_set_progress(game_achievement &value)
{
    value.set_progress(1, 2);
    return ;
}
static void game_achievement_add_progress(game_achievement &value)
{
    value.add_progress(1, 2);
    return ;
}
static void game_achievement_is_complete(game_achievement &value)
{
    (void)value.is_complete();
    return ;
}
static void game_achievement_get_error(game_achievement &value)
{
    (void)value.get_error();
    return ;
}

FT_TEST(test_game_achievement_get_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_get_id));
    return (1);
}
FT_TEST(test_game_achievement_set_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_set_id));
    return (1);
}
FT_TEST(test_game_achievement_get_goals_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_get_goals));
    return (1);
}
FT_TEST(test_game_achievement_get_goal_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_get_goal));
    return (1);
}
FT_TEST(test_game_achievement_set_goal_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_set_goal));
    return (1);
}
FT_TEST(test_game_achievement_get_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_get_progress));
    return (1);
}
FT_TEST(test_game_achievement_set_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_set_progress));
    return (1);
}
FT_TEST(test_game_achievement_add_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_add_progress));
    return (1);
}
FT_TEST(test_game_achievement_is_complete_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_is_complete));
    return (1);
}
FT_TEST(test_game_achievement_get_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_get_error));
    return (1);
}
