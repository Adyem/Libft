#include "../test_internal.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_achievement_goal_updates, "ft_achievement setters store and update goal values")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(1, 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_goal(2, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(5, achievement.get_goal(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(3, achievement.get_goal(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.set_progress(1, 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(2, achievement.get_progress(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    achievement.add_progress(1, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(5, achievement.get_progress(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT(achievement.is_goal_complete(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT(!achievement.is_goal_complete(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_goal_progress_accumulates, "ft_goal progress manipulation works piecewise")
{
    ft_goal goal;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.set_target(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.set_progress(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.add_progress(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT_EQ(2, goal.get_progress());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    goal.add_progress(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT_EQ(4, goal.get_progress());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    FT_ASSERT(goal.get_progress() == goal.get_target());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.get_error());
    return (1);
}
