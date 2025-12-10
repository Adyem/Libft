#include "../../Game/game_achievement.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_goal_default_state, "Game: goals start with zeroed progress and success error")
{
    ft_goal goal;

    FT_ASSERT_EQ(0, goal.get_target());
    FT_ASSERT_EQ(0, goal.get_progress());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, goal.get_error());
    return (1);
}

FT_TEST(test_goal_set_target_and_progress, "Game: goal setters store provided values")
{
    ft_goal goal;

    goal.set_target(5);
    goal.set_progress(2);
    FT_ASSERT_EQ(5, goal.get_target());
    FT_ASSERT_EQ(2, goal.get_progress());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, goal.get_error());
    return (1);
}

FT_TEST(test_goal_add_progress_accumulates, "Game: adding progress increases stored progress")
{
    ft_goal goal;

    goal.set_progress(3);
    goal.add_progress(4);
    FT_ASSERT_EQ(7, goal.get_progress());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, goal.get_error());
    return (1);
}

FT_TEST(test_goal_copy_preserves_values, "Game: copied goals keep target and progress")
{
    ft_goal original;
    ft_goal duplicate;

    original.set_target(12);
    original.set_progress(6);
    duplicate = original;
    FT_ASSERT_EQ(12, duplicate.get_target());
    FT_ASSERT_EQ(6, duplicate.get_progress());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, duplicate.get_error());
    return (1);
}

FT_TEST(test_goal_move_resets_source, "Game: moved goals clear source fields")
{
    ft_goal original;
    ft_goal destination;

    original.set_target(9);
    original.set_progress(4);
    destination = ft_move(original);
    FT_ASSERT_EQ(9, destination.get_target());
    FT_ASSERT_EQ(4, destination.get_progress());
    FT_ASSERT_EQ(0, original.get_target());
    FT_ASSERT_EQ(0, original.get_progress());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}

FT_TEST(test_achievement_default_state, "Game: achievements start without goals and id zero")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(0, achievement.get_id());
    FT_ASSERT_EQ(0, achievement.get_goals().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_id, "Game: achievement identifier setter works")
{
    ft_achievement achievement;

    achievement.set_id(7);
    FT_ASSERT_EQ(7, achievement.get_id());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_goal_inserts_entry, "Game: setting a goal creates an entry with target")
{
    ft_achievement achievement;

    achievement.set_goal(3, 15);
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(15, achievement.get_goal(3));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_goal_overwrites_target, "Game: updating existing goal replaces the target")
{
    ft_achievement achievement;

    achievement.set_goal(5, 10);
    achievement.set_goal(5, 20);
    FT_ASSERT_EQ(20, achievement.get_goal(5));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_progress_creates_goal, "Game: setting progress for missing goal inserts it")
{
    ft_achievement achievement;

    achievement.set_progress(2, 8);
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(8, achievement.get_progress(2));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_set_progress_updates_existing_goal, "Game: setting progress updates stored value")
{
    ft_achievement achievement;

    achievement.set_goal(4, 10);
    achievement.set_progress(4, 6);
    FT_ASSERT_EQ(6, achievement.get_progress(4));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_add_progress_creates_goal, "Game: adding progress inserts missing goals with progress")
{
    ft_achievement achievement;

    achievement.add_progress(9, 3);
    FT_ASSERT_EQ(1, achievement.get_goals().size());
    FT_ASSERT_EQ(3, achievement.get_progress(9));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_add_progress_updates_existing_goal, "Game: adding progress increases existing goal progress")
{
    ft_achievement achievement;

    achievement.set_goal(1, 5);
    achievement.set_progress(1, 2);
    achievement.add_progress(1, 4);
    FT_ASSERT_EQ(6, achievement.get_progress(1));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_get_progress_invalid_id_sets_error, "Game: progress retrieval validates identifiers")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(0, achievement.get_progress(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_get_progress_missing_goal_sets_error, "Game: requesting missing goal marks not found")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(0, achievement.get_progress(14));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_goal_complete_checks_target, "Game: goal completion requires progress meeting target")
{
    ft_achievement achievement;

    achievement.set_goal(11, 10);
    achievement.set_progress(11, 5);
    FT_ASSERT_EQ(false, achievement.is_goal_complete(11));
    achievement.add_progress(11, 5);
    FT_ASSERT_EQ(true, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_goal_complete_handles_missing_goal, "Game: completion check on missing goal reports not found")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(false, achievement.is_goal_complete(22));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_complete_false_when_any_goal_incomplete, "Game: achievements remain incomplete until all goals ready")
{
    ft_achievement achievement;

    achievement.set_goal(3, 4);
    achievement.set_progress(3, 4);
    achievement.set_goal(7, 2);
    achievement.set_progress(7, 1);
    FT_ASSERT_EQ(false, achievement.is_complete());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_is_complete_true_when_all_goals_done, "Game: achievements report complete after all goals satisfied")
{
    ft_achievement achievement;

    achievement.set_goal(3, 4);
    achievement.set_progress(3, 4);
    achievement.set_goal(7, 2);
    achievement.set_progress(7, 2);
    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, achievement.get_error());
    return (1);
}

FT_TEST(test_achievement_copy_keeps_goals, "Game: copying achievements duplicates goal progress")
{
    ft_achievement original;
    ft_achievement duplicate;

    original.set_goal(6, 9);
    original.set_progress(6, 6);
    duplicate = original;
    FT_ASSERT_EQ(1, duplicate.get_goals().size());
    FT_ASSERT_EQ(9, duplicate.get_goal(6));
    FT_ASSERT_EQ(6, duplicate.get_progress(6));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, duplicate.get_error());
    return (1);
}

FT_TEST(test_achievement_move_clears_source_goals, "Game: moving achievements transfers goals and resets origin")
{
    ft_achievement original;
    ft_achievement destination;

    original.set_goal(4, 12);
    original.set_progress(4, 10);
    destination = ft_move(original);
    FT_ASSERT_EQ(1, destination.get_goals().size());
    FT_ASSERT_EQ(12, destination.get_goal(4));
    FT_ASSERT_EQ(10, destination.get_progress(4));
    FT_ASSERT_EQ(0, original.get_goals().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}
