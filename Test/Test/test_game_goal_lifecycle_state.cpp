#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_goal_move(game_goal &value)
{
    (void)value.move(value);
    return ;
}
static void game_goal_initialize_copy(game_goal &value)
{
    (void)value.initialize(value);
    return ;
}
static void game_goal_destroy(game_goal &value)
{
    (void)value.destroy();
    return ;
}
static void game_goal_disable(game_goal &value)
{
    (void)value.disable_thread_safety();
    return ;
}
static void game_goal_get_target_again(game_goal &value)
{
    (void)value.get_target();
    return ;
}
static void game_goal_set_target_again(game_goal &value)
{
    value.set_target(9);
    return ;
}
static void game_goal_get_progress_again(game_goal &value)
{
    (void)value.get_progress();
    return ;
}
static void game_goal_set_progress_again(game_goal &value)
{
    value.set_progress(9);
    return ;
}
static void game_goal_add_progress_again(game_goal &value)
{
    value.add_progress(2);
    return ;
}
static void game_goal_error_str_again(game_goal &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_goal_move_self_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_goal>(game_goal_move));
    return (1);
}
FT_TEST(test_game_goal_initialize_copy_self_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_initialize_copy));
    return (1);
}
FT_TEST(test_game_goal_destroy_is_non_aborting)
{
    FT_ASSERT_EQ(0,
                 expect_game_lifecycle_sigabrt<game_goal>(game_goal_destroy));
    return (1);
}
FT_TEST(test_game_goal_disable_thread_safety_is_safe)
{
    FT_ASSERT_EQ(0,
                 expect_game_lifecycle_sigabrt<game_goal>(game_goal_disable));
    return (1);
}
FT_TEST(test_game_goal_second_get_target_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(
                        game_goal_get_target_again));
    return (1);
}
FT_TEST(test_game_goal_second_set_target_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(
                        game_goal_set_target_again));
    return (1);
}
FT_TEST(test_game_goal_second_get_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(
                        game_goal_get_progress_again));
    return (1);
}
FT_TEST(test_game_goal_second_set_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(
                        game_goal_set_progress_again));
    return (1);
}
FT_TEST(test_game_goal_second_add_progress_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(
                        game_goal_add_progress_again));
    return (1);
}
FT_TEST(test_game_goal_second_error_str_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_error_str_again));
    return (1);
}
