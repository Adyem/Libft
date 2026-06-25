#include "../test_internal.hpp"
#include "../../Modules/Game/game_progress_tracker.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int setup_achievement_with_goal(game_achievement &achievement, int achievement_id, int goal_id,
        int target)
{
    int initialize_result;

    initialize_result = achievement.initialize();
    if (initialize_result != FT_ERR_SUCCESS)
        return (initialize_result);
    achievement.set_id(achievement_id);
    achievement.set_goal(goal_id, target);
    return (FT_ERR_SUCCESS);
}

static int setup_basic_quest(game_quest &quest, int quest_id, int phases)
{
    int initialize_result;

    initialize_result = quest.initialize();
    if (initialize_result != FT_ERR_SUCCESS)
        return (initialize_result);
    quest.set_id(quest_id);
    quest.set_phases(phases);
    quest.set_current_phase(0);
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_progress_tracker_achievement_completion)
{
    game_progress_tracker tracker;
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        setup_achievement_with_goal(achievement, 10, 1, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_achievement(achievement));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.add_goal_progress(10, 1, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.update_goal_progress(10, 1, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT(tracker.is_achievement_complete(10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_missing_achievement)
{
    game_progress_tracker tracker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, tracker.update_goal_target(99, 1, 5));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_quest_progression)
{
    game_progress_tracker tracker;
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, setup_basic_quest(quest, 3, 2));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT(tracker.is_quest_complete(3) == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT(tracker.is_quest_complete(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_over_advance_error)
{
    game_progress_tracker tracker;
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, setup_basic_quest(quest, 8, 1));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.register_quest(quest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.advance_quest_phase(8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, tracker.advance_quest_phase(8));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, tracker.get_error());
    return (1);
}

FT_TEST(test_progress_tracker_register_achievement_single_global_error)
{
    game_progress_tracker tracker;
    int register_result;

    game_achievement achievement;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, setup_achievement_with_goal(achievement, 2, 1, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result = tracker.register_achievement(achievement));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    return (1);
}
