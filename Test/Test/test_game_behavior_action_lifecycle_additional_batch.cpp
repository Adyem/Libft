#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void action_initialize_twice(game_behavior_action &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_behavior_action_initialize_twice_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_action>(
                        action_initialize_twice));
    return (1);
}

FT_TEST(test_game_behavior_action_default_initialize_has_zero_values)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_action_id());
    FT_ASSERT_EQ(0.0, value.get_weight());
    FT_ASSERT_EQ(0.0, value.get_cooldown_seconds());
    return (1);
}

FT_TEST(test_game_behavior_action_destroy_twice_is_safe_again)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_action_reinitialize_after_destroy_again)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(3, 1.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_action_id());
    return (1);
}

FT_TEST(test_game_behavior_action_thread_safety_cycle_again)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_behavior_action_setters_preserve_values)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_action_id(-4);
    value.set_weight(2.5);
    value.set_cooldown_seconds(8.0);
    FT_ASSERT_EQ(-4, value.get_action_id());
    FT_ASSERT_EQ(2.5, value.get_weight());
    FT_ASSERT_EQ(8.0, value.get_cooldown_seconds());
    return (1);
}

FT_TEST(test_game_behavior_action_error_after_initialization)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_behavior_action_self_move_preserves_state)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(9, 3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(9, value.get_action_id());
    FT_ASSERT_EQ(3.0, value.get_weight());
    return (1);
}
