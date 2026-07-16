#include "../test_internal.hpp"

#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_progress_tracker.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_progress_tracker_thread_safe_lifecycle)
{
    game_progress_tracker tracker;
    game_achievement achievement;
    game_goal goal;
    game_quest quest;
    ft_string description;
    ft_string objective;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    goal.set_target(3);
    goal.set_progress(1);
    achievement.set_id(21);
    achievement.set_goal(7, 3);
    achievement.set_progress(7, 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_achievement(achievement));
    FT_ASSERT_EQ(FT_FALSE, tracker.is_achievement_complete(21));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.add_goal_progress(21, 7, 2));
    FT_ASSERT_EQ(FT_TRUE, tracker.is_achievement_complete(21));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("first steps"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, objective.initialize("reach the gate"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_id(14);
    quest.set_phases(2);
    quest.set_current_phase(0);
    quest.set_description(description);
    quest.set_objective(objective);
    quest.set_reward_experience(250);
    quest.set_current_phase(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(FT_FALSE, tracker.is_quest_complete(14));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(14));
    FT_ASSERT_EQ(FT_TRUE, tracker.is_quest_complete(14));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, objective.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.enable_thread_safety());
    FT_ASSERT(tracker.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.destroy());
    return (1);
}
