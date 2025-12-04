#include "../../Game/game_achievement.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_achievement_rejects_negative_goal_id, "Game achievement refuses negative goal identifiers")
{
    ft_achievement achievement;

    achievement.set_goal(-5, 10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_goal(1));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}


FT_TEST(test_game_achievement_progress_creation_for_new_goal, "Game achievement creates goal when progress assigned")
{
    ft_achievement achievement;

    achievement.set_progress(2, 6);
    FT_ASSERT_EQ(0, achievement.get_goal(2));
    FT_ASSERT_EQ(6, achievement.get_progress(2));
    FT_ASSERT_EQ(true, achievement.is_goal_complete(2));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_achievement_complete_with_no_goals, "Game achievement treats empty goal set as complete")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_achievement_rejects_negative_progress_goal, "Game achievement add_progress blocks negative goal identifiers")
{
    ft_achievement achievement;

    achievement.add_progress(-3, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(false, achievement.is_goal_complete(1));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}


FT_TEST(test_game_goal_handles_negative_delta, "Game goal supports reducing progress with negative deltas")
{
    ft_goal goal;

    goal.set_target(5);
    goal.set_progress(3);
    goal.add_progress(-2);
    FT_ASSERT_EQ(1, goal.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, goal.get_error());
    return (1);
}


FT_TEST(test_game_achievement_completion_requires_all_goals, "Game achievement completes only when all goals finish")
{
    ft_achievement achievement;

    achievement.set_goal(10, 5);
    achievement.set_goal(11, 2);
    achievement.set_progress(10, 5);
    achievement.set_progress(11, 1);
    FT_ASSERT_EQ(false, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(11, 1);
    FT_ASSERT_EQ(true, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_goal_accumulates_progress, "Game goal accumulates progress toward its target")
{
    ft_goal goal;

    goal.set_target(4);
    goal.set_progress(1);
    goal.add_progress(2);
    FT_ASSERT_EQ(3, goal.get_progress());
    goal.add_progress(1);
    FT_ASSERT_EQ(4, goal.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, goal.get_error());
    return (1);
}


FT_TEST(test_game_achievement_updates_existing_goal_target, "Game achievement updates goal targets without resetting progress")
{
    ft_achievement achievement;

    achievement.set_goal(12, 3);
    achievement.set_progress(12, 2);
    achievement.set_goal(12, 5);
    FT_ASSERT_EQ(5, achievement.get_goal(12));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(2, achievement.get_progress(12));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_achievement_rejects_negative_progress_updates, "Game achievement preserves existing goals after invalid progress id")
{
    ft_achievement achievement;

    achievement.set_progress(13, 3);
    achievement.set_progress(-2, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(3, achievement.get_progress(13));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_achievement_reports_missing_goal_completion, "Game achievement reports missing goals as incomplete")
{
    ft_achievement achievement;

    achievement.set_goal(14, 2);
    achievement.set_progress(14, 2);
    FT_ASSERT_EQ(false, achievement.is_goal_complete(15));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    FT_ASSERT_EQ(true, achievement.is_goal_complete(14));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}


FT_TEST(test_game_goal_copy_preserves_values, "Game goal copies retain progress and target")
{
    ft_goal original;

    original.set_target(7);
    original.set_progress(3);
    ft_goal copied(original);
    FT_ASSERT_EQ(7, copied.get_target());
    FT_ASSERT_EQ(3, copied.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_error());
    return (1);
}


FT_TEST(test_game_goal_assignment_overwrites_previous_values, "Game goal copy assignment replaces target and progress")
{
    ft_goal source;
    ft_goal destination;

    source.set_target(6);
    source.set_progress(5);
    destination.set_target(1);
    destination.set_progress(0);
    destination = source;
    FT_ASSERT_EQ(6, destination.get_target());
    FT_ASSERT_EQ(5, destination.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(6, source.get_target());
    FT_ASSERT_EQ(5, source.get_progress());
    return (1);
}


FT_TEST(test_game_goal_move_resets_source, "Game goal move assignment transfers values and clears origin")
{
    ft_goal source;
    ft_goal destination;

    source.set_target(9);
    source.set_progress(4);
    destination = ft_move(source);
    FT_ASSERT_EQ(9, destination.get_target());
    FT_ASSERT_EQ(4, destination.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(0, source.get_target());
    FT_ASSERT_EQ(0, source.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_achievement_move_assignment_clears_goals, "Game achievement move assignment transfers goals and resets origin")
{
    ft_achievement source;
    ft_achievement destination;

    source.set_id(21);
    source.set_goal(2, 6);
    source.set_progress(2, 3);
    destination.set_id(4);
    destination.set_goal(1, 2);

    destination = ft_move(source);
    FT_ASSERT_EQ(21, destination.get_id());
    FT_ASSERT_EQ(6, destination.get_goal(2));
    FT_ASSERT_EQ(3, destination.get_progress(2));
    FT_ASSERT_EQ(false, destination.is_goal_complete(2));
    FT_ASSERT_EQ(0, source.get_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_goal(2));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}


