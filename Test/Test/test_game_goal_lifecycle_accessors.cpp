#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_goal_get_target(game_goal &value)
{
    (void)value.get_target();
    return ;
}
static void game_goal_set_target(game_goal &value)
{
    value.set_target(7);
    return ;
}
static void game_goal_get_progress(game_goal &value)
{
    (void)value.get_progress();
    return ;
}
static void game_goal_set_progress(game_goal &value)
{
    value.set_progress(3);
    return ;
}
static void game_goal_add_progress(game_goal &value)
{
    value.add_progress(1);
    return ;
}
static void game_goal_get_error(game_goal &value)
{
    (void)value.get_error();
    return ;
}
static void game_goal_get_error_str(game_goal &value)
{
    (void)value.get_error_str();
    return ;
}
static void game_goal_enable(game_goal &value)
{
    (void)value.enable_thread_safety();
    return ;
}
static void game_goal_is_thread_safe(game_goal &value)
{
    (void)value.is_thread_safe();
    return ;
}
static void game_goal_lock(game_goal &value)
{
    ft_bool lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}

FT_TEST(test_game_goal_get_target_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_get_target));
    return (1);
}
FT_TEST(test_game_goal_set_target_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_set_target));
    return (1);
}
FT_TEST(test_game_goal_get_progress_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_get_progress));
    return (1);
}
FT_TEST(test_game_goal_set_progress_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_set_progress));
    return (1);
}
FT_TEST(test_game_goal_add_progress_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_add_progress));
    return (1);
}
FT_TEST(test_game_goal_get_error_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_goal>(game_goal_get_error));
    return (1);
}
FT_TEST(test_game_goal_get_error_str_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_get_error_str));
    return (1);
}
FT_TEST(test_game_goal_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_enable));
    return (1);
}
FT_TEST(test_game_goal_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_goal>(game_goal_is_thread_safe));
    return (1);
}
FT_TEST(test_game_goal_lock_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_goal>(game_goal_lock));
    return (1);
}
