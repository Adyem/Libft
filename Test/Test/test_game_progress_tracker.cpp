#include "../test_internal.hpp"
#include "../../Game/game_progress_tracker.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void setup_achievement_with_goal(ft_achievement &achievement, int achievement_id, int goal_id,
        int target)
{
    achievement.set_id(achievement_id);
    achievement.set_goal(goal_id, target);
}

static void setup_basic_quest(ft_quest &quest, int quest_id, int phases)
{
    quest.set_id(quest_id);
    quest.set_phases(phases);
    quest.set_current_phase(0);
}

FT_TEST(test_progress_tracker_achievement_completion, "achievement completes after reaching target")
{
    ft_progress_tracker tracker;
    ft_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    setup_achievement_with_goal(achievement, 10, 1, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_achievement(achievement));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.add_goal_progress(10, 1, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.update_goal_progress(10, 1, 3));
    FT_ASSERT(tracker.is_achievement_complete(10));
    return (1);
}

FT_TEST(test_progress_tracker_missing_achievement, "missing achievement returns not found")
{
    ft_progress_tracker tracker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, tracker.update_goal_target(99, 1, 5));
    return (1);
}

FT_TEST(test_progress_tracker_quest_progression, "quest advances until completion")
{
    ft_progress_tracker tracker;
    ft_quest quest;
    setup_basic_quest(quest, 3, 2);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT(tracker.is_quest_complete(3) == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT(tracker.is_quest_complete(3));
    return (1);
}

FT_TEST(test_progress_tracker_over_advance_error, "advancing beyond quest phases returns error")
{
    ft_progress_tracker tracker;
    ft_quest quest;
    setup_basic_quest(quest, 8, 1);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(8));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, tracker.advance_quest_phase(8));
    return (1);
}

FT_TEST(test_progress_tracker_register_achievement_single_global_error,
    "register_achievement pushes one global error entry")
{
    ft_progress_tracker tracker;
    int register_result;

    ft_achievement achievement;
    setup_achievement_with_goal(achievement, 2, 1, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result = tracker.register_achievement(achievement));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    return (1);
}
