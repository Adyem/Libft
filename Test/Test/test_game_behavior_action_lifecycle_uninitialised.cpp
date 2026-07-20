#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_behavior_action_get_action_id(game_behavior_action &value)
{
    (void)value.get_action_id();
    return ;
}

static void game_behavior_action_set_action_id(game_behavior_action &value)
{
    value.set_action_id(1);
    return ;
}

static void game_behavior_action_get_weight(game_behavior_action &value)
{
    (void)value.get_weight();
    return ;
}

static void game_behavior_action_set_weight(game_behavior_action &value)
{
    value.set_weight(1.0);
    return ;
}

static void game_behavior_action_get_cooldown_seconds(game_behavior_action &value)
{
    (void)value.get_cooldown_seconds();
    return ;
}

static void game_behavior_action_set_cooldown_seconds(game_behavior_action &value)
{
    value.set_cooldown_seconds(1.0);
    return ;
}

static void game_behavior_action_get_error(game_behavior_action &value)
{
    (void)value.get_error();
    return ;
}

static void game_behavior_action_get_error_str(game_behavior_action &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_behavior_action_enable_thread_safety(game_behavior_action &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_behavior_action_disable_thread_safety(game_behavior_action &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_behavior_action_is_thread_safe(game_behavior_action &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_behavior_action_get_action_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_get_action_id));
    return (1);
}

FT_TEST(test_game_behavior_action_set_action_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_set_action_id));
    return (1);
}

FT_TEST(test_game_behavior_action_get_weight_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_get_weight));
    return (1);
}

FT_TEST(test_game_behavior_action_set_weight_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_set_weight));
    return (1);
}

FT_TEST(test_game_behavior_action_get_cooldown_seconds_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_get_cooldown_seconds));
    return (1);
}

FT_TEST(test_game_behavior_action_set_cooldown_seconds_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_set_cooldown_seconds));
    return (1);
}

FT_TEST(test_game_behavior_action_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_get_error));
    return (1);
}

FT_TEST(test_game_behavior_action_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_get_error_str));
    return (1);
}

FT_TEST(test_game_behavior_action_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_behavior_action_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_behavior_action_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_action>(
        game_behavior_action_is_thread_safe));
    return (1);
}
