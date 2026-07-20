#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_achievement_additional_get_goals_const(game_achievement &value)
{
    const game_achievement &const_value = value;

    (void)const_value.get_goals();
    return ;
}

static void game_achievement_additional_set_goals(game_achievement &value)
{
    ft_map<int32_t, game_goal> goals;

    (void)goals.initialize();
    value.set_goals(goals);
    return ;
}

static void game_achievement_additional_lock(game_achievement &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void game_achievement_additional_unlock(game_achievement &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void
game_achievement_additional_initialize_twice(game_achievement &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_achievement_additional_copy_initialize_uninitialised(
    game_achievement &value)
{
    game_achievement source;

    (void)value.initialize(source);
    return ;
}

static void game_achievement_additional_move_initialize_uninitialised(
    game_achievement &value)
{
    game_achievement source;

    (void)value.initialize(static_cast<game_achievement &&>(source));
    return ;
}

static void
game_achievement_additional_move_uninitialised(game_achievement &value)
{
    game_achievement source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_achievement_get_goals_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_get_goals_const));
    return (1);
}

FT_TEST(test_game_achievement_set_goals_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_set_goals));
    return (1);
}

FT_TEST(test_game_achievement_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_lock));
    return (1);
}

FT_TEST(test_game_achievement_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_unlock));
    return (1);
}

FT_TEST(test_game_achievement_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_achievement_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_achievement>(
               game_achievement_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_achievement_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_achievement>(
               game_achievement_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_achievement_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_achievement>(
                        game_achievement_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_achievement_get_goals_mutable_initialised)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_goals().size());
    return (1);
}

FT_TEST(test_game_achievement_get_goals_const_initialised)
{
    game_achievement value;
    const game_achievement &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_goals().size());
    return (1);
}

FT_TEST(test_game_achievement_set_empty_goals_initialised)
{
    game_achievement value;
    ft_map<int32_t, game_goal> goals;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goals.initialize());
    value.set_goals(goals);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_EQ(0, value.get_goals().size());
    return (1);
}

FT_TEST(test_game_achievement_empty_achievement_is_complete)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_TRUE, value.is_complete());
    return (1);
}

FT_TEST(test_game_achievement_goal_completion_before_target)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_goal(2, 10);
    value.set_progress(2, 9);
    FT_ASSERT_EQ(FT_FALSE, value.is_goal_complete(2));
    return (1);
}

FT_TEST(test_game_achievement_goal_completion_at_target)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_goal(2, 10);
    value.set_progress(2, 10);
    FT_ASSERT_EQ(FT_TRUE, value.is_goal_complete(2));
    return (1);
}

FT_TEST(test_game_achievement_all_goals_complete)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_goal(1, 3);
    value.set_goal(2, 5);
    value.set_progress(1, 3);
    value.set_progress(2, 5);
    FT_ASSERT_EQ(FT_TRUE, value.is_complete());
    return (1);
}

FT_TEST(test_game_achievement_get_goal_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_goal(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_get_progress_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_progress(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_set_goal_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_goal(-1, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_set_progress_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_progress(-1, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_add_progress_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.add_progress(-1, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_is_goal_complete_negative_id_reports_error)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_goal_complete(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_achievement_thread_safety_enable_disable_cycle)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_achievement_lock_unlock_initialised)
{
    game_achievement value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_achievement_destroy_twice_is_safe)
{
    game_achievement value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
