#include "../../Modules/Game/game_scripting_bridge.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void script_context_initialize_twice(game_script_context &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_script_context_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_script_context>(
                        script_context_initialize_twice));
    return (1);
}

FT_TEST(test_game_script_context_empty_variable_is_not_found)
{
    game_script_context value;
    ft_string key;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("missing"));
    FT_ASSERT_EQ(ft_nullptr, value.get_variable(key));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_and_get_variable)
{
    game_script_context value;
    ft_string key;
    ft_string variable_value;
    const ft_string *found_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("name"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variable_value.initialize("Alex"));
    value.set_variable(key, variable_value);
    found_value = value.get_variable(key);
    FT_ASSERT_NEQ(ft_nullptr, found_value);
    FT_ASSERT_STR_EQ("Alex", found_value->c_str());
    return (1);
}

FT_TEST(test_game_script_context_remove_missing_variable_is_safe)
{
    game_script_context value;
    ft_string key;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("missing"));
    value.remove_variable(key);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_script_context_clear_variables_is_idempotent)
{
    game_script_context value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.clear_variables();
    value.clear_variables();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_variable_replaces_value)
{
    game_script_context value;
    ft_string key;
    ft_string first_value;
    ft_string second_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("name"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_value.initialize("first"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_value.initialize("second"));
    value.set_variable(key, first_value);
    value.set_variable(key, second_value);
    FT_ASSERT_STR_EQ("second", value.get_variable(key)->c_str());
    return (1);
}

FT_TEST(test_game_script_context_default_state_pointer_is_null)
{
    game_script_context value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(ft_nullptr, value.get_state());
    return (1);
}

FT_TEST(test_game_script_context_set_state_accepts_null)
{
    game_script_context value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_state(ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, value.get_state());
    return (1);
}

FT_TEST(test_game_script_context_set_world_accepts_empty_pointer)
{
    game_script_context value;
    ft_sharedptr<game_world> world_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_world(world_pointer);
    FT_ASSERT_EQ(ft_nullptr, value.get_world().get());
    return (1);
}

FT_TEST(test_game_script_context_destroy_twice_is_safe)
{
    game_script_context value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_script_context_reinitialize_after_destroy)
{
    game_script_context value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(ft_nullptr, value.get_state());
    return (1);
}
