#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_behavior_profile_additional_get_actions_const(game_behavior_profile &value)
{
    const game_behavior_profile &const_value = value;

    (void)const_value.get_actions();
    return ;
}

static void
game_behavior_profile_additional_set_actions(game_behavior_profile &value)
{
    ft_vector<game_behavior_action> actions;

    (void)actions.initialize();
    value.set_actions(actions);
    return ;
}

static void game_behavior_profile_additional_lock(game_behavior_profile &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void
game_behavior_profile_additional_unlock(game_behavior_profile &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void
game_behavior_profile_additional_get_error_str(game_behavior_profile &value)
{
    (void)value.get_error_str();
    return ;
}

static void
game_behavior_profile_additional_initialize_twice(game_behavior_profile &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_behavior_profile_additional_copy_initialize_uninitialised(
    game_behavior_profile &value)
{
    game_behavior_profile source;

    (void)value.initialize(source);
    return ;
}

static void game_behavior_profile_additional_move_initialize_uninitialised(
    game_behavior_profile &value)
{
    game_behavior_profile source;

    (void)value.initialize(static_cast<game_behavior_profile &&>(source));
    return ;
}

static void game_behavior_profile_additional_move_uninitialised(
    game_behavior_profile &value)
{
    game_behavior_profile source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_behavior_profile_get_actions_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_get_actions_const));
    return (1);
}

FT_TEST(test_game_behavior_profile_set_actions_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_set_actions));
    return (1);
}

FT_TEST(test_game_behavior_profile_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_lock));
    return (1);
}

FT_TEST(test_game_behavior_profile_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_unlock));
    return (1);
}

FT_TEST(test_game_behavior_profile_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_get_error_str));
    return (1);
}

FT_TEST(test_game_behavior_profile_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_behavior_profile_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
               game_behavior_profile_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_profile_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
               game_behavior_profile_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_profile_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_profile>(
                        game_behavior_profile_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_profile_initialize_default_values)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.0, value.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, value.get_caution_weight());
    FT_ASSERT_EQ(0, value.get_actions().size());
    return (1);
}

FT_TEST(test_game_behavior_profile_initialize_with_values)
{
    game_behavior_profile value;
    ft_vector<game_behavior_action> actions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(4, 0.7, 0.3, actions));
    FT_ASSERT_EQ(4, value.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.7, value.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.3, value.get_caution_weight());
    FT_ASSERT_EQ(0, value.get_actions().size());
    return (1);
}

FT_TEST(test_game_behavior_profile_set_profile_id_initialised)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_profile_id(9);
    FT_ASSERT_EQ(9, value.get_profile_id());
    return (1);
}

FT_TEST(test_game_behavior_profile_set_aggression_initialised)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_aggression_weight(0.8);
    FT_ASSERT_DOUBLE_EQ(0.8, value.get_aggression_weight());
    return (1);
}

FT_TEST(test_game_behavior_profile_set_caution_initialised)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_caution_weight(0.2);
    FT_ASSERT_DOUBLE_EQ(0.2, value.get_caution_weight());
    return (1);
}

FT_TEST(test_game_behavior_profile_get_actions_mutable_initialised)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_actions().size());
    return (1);
}

FT_TEST(test_game_behavior_profile_get_actions_const_initialised)
{
    game_behavior_profile value;
    const game_behavior_profile &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_actions().size());
    return (1);
}

FT_TEST(test_game_behavior_profile_set_empty_actions_initialised)
{
    game_behavior_profile value;
    ft_vector<game_behavior_action> actions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.initialize());
    value.set_actions(actions);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_EQ(0, value.get_actions().size());
    return (1);
}

FT_TEST(test_game_behavior_profile_thread_safety_enable_disable_cycle)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_behavior_profile_lock_unlock_initialised)
{
    game_behavior_profile value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_behavior_profile_destroy_twice_is_safe)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_profile_get_error_after_success)
{
    game_behavior_profile value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_behavior_profile_get_error_str_after_success)
{
    game_behavior_profile value;
    const char *error_string;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    error_string = value.get_error_str();
    FT_ASSERT_NEQ(ft_nullptr, error_string);
    return (1);
}
