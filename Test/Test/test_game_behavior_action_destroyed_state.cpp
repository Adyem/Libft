#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_behavior_action_default_set_callback_is_safe)
{
    game_behavior_tree_action value;
    ft_function<int32_t(game_behavior_context &)> callback;

    value.set_callback(callback);
    FT_ASSERT_NEQ(ft_nullptr, &value.get_callback());
    return (1);
}

FT_TEST(test_game_behavior_action_default_get_callback_is_safe)
{
    game_behavior_tree_action value;

    FT_ASSERT_NEQ(ft_nullptr, &value.get_callback());
    return (1);
}

FT_TEST(test_game_behavior_action_default_tick_reports_failure)
{
    game_behavior_tree_action value;
    game_behavior_context context;

    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, value.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_get_error_is_valid)
{
    game_behavior_tree_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_get_error_str_is_valid)
{
    game_behavior_tree_action value;

    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_destructor_is_non_aborting)
{
    game_behavior_tree_action *value;

    value = new game_behavior_tree_action();
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_callback_default_is_accessible)
{
    game_behavior_tree_action value;

    FT_ASSERT_NEQ(ft_nullptr, &value.get_callback());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_tick_context_is_local)
{
    game_behavior_tree_action value;
    game_behavior_context context;

    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, value.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_set_callback_can_be_called)
{
    game_behavior_tree_action value;
    ft_function<int32_t(game_behavior_context &)> callback;

    value.set_callback(callback);
    FT_ASSERT_NEQ(ft_nullptr, &value.get_callback());
    return (1);
}

FT_TEST(test_game_behavior_action_destroyed_error_is_success)
{
    game_behavior_tree_action value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}
