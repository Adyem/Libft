#include "../test_internal.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_achievement_goal_updates, "ft_achievement setters store and update goal values")
{
    ft_achievement achievement;

    achievement.set_goal(1, 5);
    achievement.set_goal(2, 3);
    FT_ASSERT_EQ(5, achievement.get_goal(1));
    FT_ASSERT_EQ(3, achievement.get_goal(2));
    achievement.set_progress(1, 2);
    FT_ASSERT_EQ(2, achievement.get_progress(1));
    achievement.add_progress(1, 3);
    FT_ASSERT_EQ(5, achievement.get_progress(1));
    FT_ASSERT(achievement.is_goal_complete(1));
    FT_ASSERT(!achievement.is_goal_complete(2));
    return (1);
}

FT_TEST(test_game_goal_progress_accumulates, "ft_goal progress manipulation works piecewise")
{
    ft_goal goal;

    goal.set_target(4);
    goal.set_progress(1);
    goal.add_progress(1);
    FT_ASSERT_EQ(2, goal.get_progress());
    goal.add_progress(3);
    FT_ASSERT_EQ(4, goal.get_progress());
    FT_ASSERT(goal.get_progress() == goal.get_target());
    return (1);
}
