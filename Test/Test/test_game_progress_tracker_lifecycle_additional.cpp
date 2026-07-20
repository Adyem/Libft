#include "../../Modules/Game/game_progress_tracker.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void progress_initialize_twice(game_progress_tracker &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void progress_copy_initialize_uninitialised(game_progress_tracker &value)
{
    game_progress_tracker source;

    (void)value.initialize(source);
    return ;
}

static void progress_move_initialize_uninitialised(game_progress_tracker &value)
{
    game_progress_tracker source;

    (void)value.initialize(static_cast<game_progress_tracker &&>(source));
    return ;
}

static void progress_move_uninitialised(game_progress_tracker &value)
{
    game_progress_tracker source;

    (void)value.move(source);
    return ;
}

static void progress_get_achievements(game_progress_tracker &value)
{
    (void)value.get_achievements();
    return ;
}

static void progress_get_achievements_const(game_progress_tracker &value)
{
    const game_progress_tracker &const_value = value;

    (void)const_value.get_achievements();
    return ;
}

static void progress_get_quests(game_progress_tracker &value)
{
    (void)value.get_quests();
    return ;
}

static void progress_get_quests_const(game_progress_tracker &value)
{
    const game_progress_tracker &const_value = value;

    (void)const_value.get_quests();
    return ;
}

static void progress_set_achievements(game_progress_tracker &value)
{
    ft_map<int32_t, game_achievement> achievements;

    (void)achievements.initialize();
    value.set_achievements(achievements);
    return ;
}

static void progress_set_quests(game_progress_tracker &value)
{
    ft_map<int32_t, game_quest> quests;

    (void)quests.initialize();
    value.set_quests(quests);
    return ;
}

static void progress_register_achievement(game_progress_tracker &value)
{
    game_achievement achievement;

    (void)value.register_achievement(achievement);
    return ;
}

static void progress_register_quest(game_progress_tracker &value)
{
    game_quest quest;

    (void)value.register_quest(quest);
    return ;
}

static void progress_update_goal_target(game_progress_tracker &value)
{
    (void)value.update_goal_target(1, 1, 2);
    return ;
}

static void progress_update_goal_progress(game_progress_tracker &value)
{
    (void)value.update_goal_progress(1, 1, 2);
    return ;
}

static void progress_add_goal_progress(game_progress_tracker &value)
{
    (void)value.add_goal_progress(1, 1, 2);
    return ;
}

static void progress_is_achievement_complete(game_progress_tracker &value)
{
    (void)value.is_achievement_complete(1);
    return ;
}

static void progress_set_quest_phase(game_progress_tracker &value)
{
    (void)value.set_quest_phase(1, 2);
    return ;
}

static void progress_advance_quest_phase(game_progress_tracker &value)
{
    (void)value.advance_quest_phase(1);
    return ;
}

static void progress_is_quest_complete(game_progress_tracker &value)
{
    (void)value.is_quest_complete(1);
    return ;
}

static void progress_enable_thread_safety(game_progress_tracker &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void progress_lock(game_progress_tracker &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void progress_get_error(game_progress_tracker &value)
{
    (void)value.get_error();
    return ;
}

FT_TEST(test_game_progress_tracker_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_initialize_twice));
    return (1);
}

FT_TEST(test_game_progress_tracker_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_progress_tracker_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_progress_tracker_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_move_uninitialised));
    return (1);
}

FT_TEST(test_game_progress_tracker_get_achievements_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_get_achievements));
    return (1);
}

FT_TEST(test_game_progress_tracker_get_achievements_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_get_achievements_const));
    return (1);
}

FT_TEST(test_game_progress_tracker_get_quests_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_get_quests));
    return (1);
}

FT_TEST(test_game_progress_tracker_get_quests_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_get_quests_const));
    return (1);
}

FT_TEST(test_game_progress_tracker_set_achievements_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_set_achievements));
    return (1);
}

FT_TEST(test_game_progress_tracker_set_quests_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_set_quests));
    return (1);
}

FT_TEST(test_game_progress_tracker_register_achievement_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_register_achievement));
    return (1);
}

FT_TEST(test_game_progress_tracker_register_quest_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_register_quest));
    return (1);
}

FT_TEST(test_game_progress_tracker_update_goal_target_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_update_goal_target));
    return (1);
}

FT_TEST(test_game_progress_tracker_update_goal_progress_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_update_goal_progress));
    return (1);
}

FT_TEST(test_game_progress_tracker_add_goal_progress_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_add_goal_progress));
    return (1);
}

FT_TEST(test_game_progress_tracker_is_achievement_complete_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_is_achievement_complete));
    return (1);
}

FT_TEST(test_game_progress_tracker_set_quest_phase_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_set_quest_phase));
    return (1);
}

FT_TEST(test_game_progress_tracker_advance_quest_phase_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_advance_quest_phase));
    return (1);
}

FT_TEST(test_game_progress_tracker_is_quest_complete_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_is_quest_complete));
    return (1);
}

FT_TEST(test_game_progress_tracker_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_progress_tracker_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_progress_tracker>(progress_lock));
    return (1);
}

FT_TEST(test_game_progress_tracker_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_progress_tracker>(
                        progress_get_error));
    return (1);
}
