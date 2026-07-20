#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_quest_get_id(game_quest &value)
{
    (void)value.get_id();
    return ;
}

static void game_quest_set_id(game_quest &value)
{
    value.set_id(1);
    return ;
}

static void game_quest_get_phases(game_quest &value)
{
    (void)value.get_phases();
    return ;
}

static void game_quest_set_phases(game_quest &value)
{
    value.set_phases(1);
    return ;
}

static void game_quest_get_current_phase(game_quest &value)
{
    (void)value.get_current_phase();
    return ;
}

static void game_quest_set_current_phase(game_quest &value)
{
    value.set_current_phase(1);
    return ;
}

static void game_quest_get_description(game_quest &value)
{
    (void)value.get_description();
    return ;
}

static void game_quest_set_description(game_quest &value)
{
    ft_string description;

    value.set_description(description);
    return ;
}

static void game_quest_get_objective(game_quest &value)
{
    (void)value.get_objective();
    return ;
}

static void game_quest_set_objective(game_quest &value)
{
    ft_string objective;

    value.set_objective(objective);
    return ;
}

static void game_quest_get_reward_experience(game_quest &value)
{
    (void)value.get_reward_experience();
    return ;
}

static void game_quest_set_reward_experience(game_quest &value)
{
    value.set_reward_experience(1);
    return ;
}

static void game_quest_get_reward_items(game_quest &value)
{
    (void)value.get_reward_items();
    return ;
}

static void game_quest_get_reward_items_const(game_quest &value)
{
    const game_quest &constant_value = value;

    (void)constant_value.get_reward_items();
    return ;
}

static void game_quest_set_reward_items(game_quest &value)
{
    ft_vector<ft_sharedptr<game_item>> items;

    value.set_reward_items(items);
    return ;
}

static void game_quest_is_complete(game_quest &value)
{
    (void)value.is_complete();
    return ;
}

static void game_quest_advance_phase(game_quest &value)
{
    value.advance_phase();
    return ;
}

static void game_quest_enable_thread_safety(game_quest &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_quest_disable_thread_safety(game_quest &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_quest_is_thread_safe(game_quest &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_quest_get_error(game_quest &value)
{
    (void)value.get_error();
    return ;
}

static void game_quest_get_error_str(game_quest &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_quest_get_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_quest>(game_quest_get_id));
    return (1);
}

FT_TEST(test_game_quest_set_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_quest>(game_quest_set_id));
    return (1);
}

FT_TEST(test_game_quest_get_phases_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_get_phases));
    return (1);
}

FT_TEST(test_game_quest_set_phases_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_set_phases));
    return (1);
}

FT_TEST(test_game_quest_get_current_phase_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_get_current_phase));
    return (1);
}

FT_TEST(test_game_quest_set_current_phase_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_set_current_phase));
    return (1);
}

FT_TEST(test_game_quest_get_description_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_get_description));
    return (1);
}

FT_TEST(test_game_quest_set_description_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_set_description));
    return (1);
}

FT_TEST(test_game_quest_get_objective_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_get_objective));
    return (1);
}

FT_TEST(test_game_quest_set_objective_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_set_objective));
    return (1);
}

FT_TEST(test_game_quest_get_reward_experience_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_get_reward_experience));
    return (1);
}

FT_TEST(test_game_quest_set_reward_experience_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_set_reward_experience));
    return (1);
}

FT_TEST(test_game_quest_get_reward_items_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_get_reward_items));
    return (1);
}

FT_TEST(test_game_quest_get_reward_items_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_get_reward_items_const));
    return (1);
}

FT_TEST(test_game_quest_set_reward_items_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_set_reward_items));
    return (1);
}

FT_TEST(test_game_quest_is_complete_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_is_complete));
    return (1);
}

FT_TEST(test_game_quest_advance_phase_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_advance_phase));
    return (1);
}

FT_TEST(test_game_quest_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_quest_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_quest_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_is_thread_safe));
    return (1);
}

FT_TEST(test_game_quest_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_get_error));
    return (1);
}

FT_TEST(test_game_quest_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_quest>(game_quest_get_error_str));
    return (1);
}
