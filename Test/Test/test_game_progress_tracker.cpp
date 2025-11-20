#include "../../Game/game_progress_tracker.hpp"
#include "../../System_utils/test_runner.hpp"

static ft_achievement make_achievement_with_goal(int achievement_id, int goal_id, int target)
{
    ft_achievement achievement;

    achievement.set_id(achievement_id);
    achievement.set_goal(goal_id, target);
    return (achievement);
}

static ft_quest make_basic_quest(int quest_id, int phases)
{
    ft_quest quest;

    quest.set_id(quest_id);
    quest.set_phases(phases);
    quest.set_current_phase(0);
    return (quest);
}

FT_TEST(test_progress_tracker_achievement_completion, "achievement completes after reaching target")
{
    ft_progress_tracker tracker;
    ft_achievement achievement = make_achievement_with_goal(10, 1, 3);

    FT_ASSERT_EQ(ER_SUCCESS, tracker.register_achievement(achievement));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.add_goal_progress(10, 1, 1));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.update_goal_progress(10, 1, 3));
    FT_ASSERT(tracker.is_achievement_complete(10));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_missing_achievement, "missing achievement returns not found")
{
    ft_progress_tracker tracker;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, tracker.update_goal_target(99, 1, 5));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_quest_progression, "quest advances until completion")
{
    ft_progress_tracker tracker;
    ft_quest quest = make_basic_quest(3, 2);

    FT_ASSERT_EQ(ER_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT(tracker.is_quest_complete(3) == false);
    FT_ASSERT_EQ(ER_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT(tracker.is_quest_complete(3));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_over_advance_error, "advancing beyond quest phases returns error")
{
    ft_progress_tracker tracker;
    ft_quest quest = make_basic_quest(8, 1);

    FT_ASSERT_EQ(ER_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(ER_SUCCESS, tracker.advance_quest_phase(8));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, tracker.advance_quest_phase(8));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, tracker.get_error());
    return (1);
}
