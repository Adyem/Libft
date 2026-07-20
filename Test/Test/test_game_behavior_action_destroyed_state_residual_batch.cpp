#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void behavior_action_destroyed_get_action_id(game_behavior_action &value)
{
    (void)value.get_action_id();
    return ;
}

static void behavior_action_destroyed_set_action_id(game_behavior_action &value)
{
    value.set_action_id(1);
    return ;
}

static void behavior_action_destroyed_get_weight(game_behavior_action &value)
{
    (void)value.get_weight();
    return ;
}

static void behavior_action_destroyed_set_weight(game_behavior_action &value)
{
    value.set_weight(1.0);
    return ;
}

static void behavior_action_destroyed_get_cooldown(game_behavior_action &value)
{
    (void)value.get_cooldown_seconds();
    return ;
}

static void behavior_action_destroyed_set_cooldown(game_behavior_action &value)
{
    value.set_cooldown_seconds(1.0);
    return ;
}

FT_TEST(test_game_behavior_action_destroyed_get_action_id_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_get_action_id));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_set_action_id_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_set_action_id));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_get_weight_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_get_weight));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_set_weight_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_set_weight));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_get_cooldown_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_get_cooldown));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_set_cooldown_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_action>(
                        behavior_action_destroyed_set_cooldown));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_error_queries_are_valid)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_thread_safety_is_disabled)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_cleanup_can_be_repeated)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_destructor_is_non_aborting)
{
    game_behavior_action *value;

    value = new game_behavior_action();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_thread_safety_can_restart)
{
    game_behavior_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}
