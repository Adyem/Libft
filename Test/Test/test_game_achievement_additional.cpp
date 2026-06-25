#include "../test_internal.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_goal_default_state)
{
    game_goal goal;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT_EQ(0, goal.get_target());
    FT_ASSERT_EQ(0, goal.get_progress());
    return (1);
}

FT_TEST(test_goal_set_target_and_progress)
{
    game_goal goal;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.set_target(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.set_progress(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT_EQ(5, goal.get_target());
    FT_ASSERT_EQ(2, goal.get_progress());
    return (1);
}

FT_TEST(test_goal_add_progress_accumulates)
{
    game_goal goal;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.set_progress(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.add_progress(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT_EQ(7, goal.get_progress());
    return (1);
}

FT_TEST(test_goal_copy_preserves_values)
{
    game_goal original;
    game_goal duplicate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_target(12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_progress(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    FT_ASSERT_EQ(12, duplicate.get_target());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    FT_ASSERT_EQ(6, duplicate.get_progress());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    return (1);
}

FT_TEST(test_goal_move_resets_source)
{
    game_goal original;
    game_goal destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_target(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_progress(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(original)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(9, destination.get_target());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(4, destination.get_progress());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, original._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_achievement_default_state)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_goals().size());
    return (1);
}

FT_TEST(test_achievement_set_id)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_id(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(7, achievement.get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_goal_inserts_entry)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(3, 15);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(15, achievement.get_goal(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_goal_overwrites_target)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(5, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(5, 20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(20, achievement.get_goal(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_progress_creates_goal)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_progress(2, 8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(8, achievement.get_progress(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_progress_updates_existing_goal)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(4, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_progress(4, 6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(6, achievement.get_progress(4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_add_progress_creates_goal)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.add_progress(9, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(3, achievement.get_progress(9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_add_progress_updates_existing_goal)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(1, 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_progress(1, 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.add_progress(1, 4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(6, achievement.get_progress(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_get_progress_invalid_id_sets_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_progress(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_get_progress_missing_goal_sets_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_progress(14));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_goal_complete_checks_target)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(11, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_progress(11, 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(false, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.add_progress(11, 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(true, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_goal_complete_handles_missing_goal)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(false, achievement.is_goal_complete(22));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_complete_false_when_any_goal_incomplete)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    achievement.set_goal(3, 4);
    achievement.set_progress(3, 4);
    achievement.set_goal(7, 2);
    achievement.set_progress(7, 1);
    FT_ASSERT_EQ(false, achievement.is_complete());
    return (1);
}

FT_TEST(test_achievement_is_complete_true_when_all_goals_done)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    achievement.set_goal(3, 4);
    achievement.set_progress(3, 4);
    achievement.set_goal(7, 2);
    achievement.set_progress(7, 2);
    FT_ASSERT_EQ(true, achievement.is_complete());
    return (1);
}

FT_TEST(test_achievement_copy_keeps_goals)
{
    game_achievement original;
    game_achievement duplicate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    original.set_goal(6, 9);
    original.set_progress(6, 6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(original));
    FT_ASSERT_EQ(1, duplicate.get_goals().size());
    FT_ASSERT_EQ(9, duplicate.get_goal(6));
    FT_ASSERT_EQ(6, duplicate.get_progress(6));
    return (1);
}

FT_TEST(test_achievement_move_clears_source_goals)
{
    game_achievement original;
    game_achievement destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    original.set_goal(4, 12);
    original.set_progress(4, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(original)));
    FT_ASSERT_EQ(1, destination.get_goals().size());
    FT_ASSERT_EQ(12, destination.get_goal(4));
    FT_ASSERT_EQ(10, destination.get_progress(4));
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, original._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_achievement_get_goal_invalid_id_reports_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_goal(-3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_goal_invalid_id_reports_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    achievement.set_goal(-5, 10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_add_progress_invalid_id_reports_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    achievement.add_progress(-7, 2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}
