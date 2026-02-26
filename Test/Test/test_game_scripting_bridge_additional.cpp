#include "../test_internal.hpp"
#include "../../Game/game_scripting_bridge.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/shared_ptr.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int script_noop(ft_game_script_context &context, const ft_vector<ft_string> &arguments) noexcept
{
    (void)arguments;
    ft_string flag_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, flag_key.initialize("flag"));
    ft_string flag_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, flag_value.initialize("1"));
    context.set_variable(flag_key, flag_value);
    return (context.get_error());
}

static int script_set_score(ft_game_script_context &context, const ft_vector<ft_string> &arguments) noexcept
{
    if (arguments.size() == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    ft_string score_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_key.initialize("score"));
    context.set_variable(score_key, arguments[0]);
    return (context.get_error());
}

FT_TEST(test_game_script_bridge_defaults_to_lua_language, "Game: scripting bridge defaults to lua when language is null")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer, ft_nullptr));

    FT_ASSERT(world_pointer);
    FT_ASSERT_STR_EQ("lua", bridge.get_language().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(32, bridge.get_max_operations());
    return (1);
}

FT_TEST(test_game_script_bridge_rejects_unsupported_language, "Game: scripting bridge constructor rejects unsupported languages")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.initialize(world_pointer, "ruby"));

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_STR_EQ("ruby", bridge.get_language().c_str());
    return (1);
}

FT_TEST(test_game_script_bridge_language_update_accepts_supported_value, "Game: set_language updates to supported languages")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));

    {
        ft_string python_lang;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, python_lang.initialize("python"));
        bridge.set_language(python_lang);
        FT_ASSERT_STR_EQ("python", bridge.get_language().c_str());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_language_update_rejects_null, "Game: set_language rejects null and preserves previous language")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));

    {
        ft_string python_lang;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, python_lang.initialize("python"));
        bridge.set_language(python_lang);
        FT_ASSERT_STR_EQ("python", bridge.get_language().c_str());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    bridge.set_language(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_STR_EQ("python", bridge.get_language().c_str());
    return (1);
}

FT_TEST(test_game_script_bridge_callback_count_tracks_overwrite, "Game: register_function overwrites existing callbacks without growing")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> first_callback(script_noop);
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> second_callback(script_set_score);

    FT_ASSERT_EQ(0u, bridge.get_callback_count());
    ft_string do_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, do_name.initialize("do"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.register_function(do_name, first_callback));
    FT_ASSERT_EQ(1u, bridge.get_callback_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.register_function(do_name, second_callback));
    FT_ASSERT_EQ(1u, bridge.get_callback_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_register_function_rejects_empty_name, "Game: register_function rejects empty callback names")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> callback(script_noop);
    int result;

    ft_string empty_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_name.initialize(""));
    result = bridge.register_function(empty_name, callback);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_EQ(0u, bridge.get_callback_count());
    return (1);
}

FT_TEST(test_game_script_bridge_register_function_rejects_null_callback, "Game: register_function rejects missing callback target")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> callback;
    int result;

    ft_string noop_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, noop_name.initialize("noop"));
    result = bridge.register_function(noop_name, callback);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_EQ(0u, bridge.get_callback_count());
    return (1);
}

FT_TEST(test_game_script_bridge_remove_function_clears_existing_entry, "Game: remove_function deletes registered callbacks")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> callback(script_noop);

    ft_string noop_name_local;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, noop_name_local.initialize("noop"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.register_function(noop_name_local, callback));
    FT_ASSERT_EQ(1u, bridge.get_callback_count());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.remove_function(noop_name_local));
    FT_ASSERT_EQ(0u, bridge.get_callback_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_set_max_operations_updates_limit, "Game: set_max_operations updates execution budget")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));

    bridge.set_max_operations(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(5, bridge.get_max_operations());
    return (1);
}

FT_TEST(test_game_script_bridge_set_max_operations_rejects_negative, "Game: set_max_operations rejects negative limits")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));

    bridge.set_max_operations(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(7, bridge.get_max_operations());

    bridge.set_max_operations(-1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_EQ(7, bridge.get_max_operations());
    return (1);
}

FT_TEST(test_game_script_bridge_thread_safety_toggle, "Game: scripting bridge thread safety toggles explicitly")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));

    FT_ASSERT_EQ(false, bridge.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(true, bridge.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(false, bridge.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_execute_reports_unknown_commands, "Game: execute reports invalid command errors")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("unknown jump\n"));
    int result;

    result = bridge.execute(script, state);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_execute_with_registered_callback_runs_successfully, "Game: execute runs registered callbacks with arguments")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> callback(script_set_score);
    const ft_string *score_value;
    ft_string script;

    script = "call score 42\n";
    ft_string score_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_name.initialize("score"));
    bridge.register_function(score_name, callback);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.execute(script, state));

    score_value = state.get_variable(score_name);
    FT_ASSERT(score_value != ft_nullptr);
    FT_ASSERT_STR_EQ("42", score_value->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    return (1);
}

FT_TEST(test_game_script_bridge_validate_dry_run_counts_operations, "Game: validate_dry_run counts non-comment lines without errors")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_vector<ft_string> warnings;
    ft_string script;
    int result;

    script = "# comment\nset score 1\n;another\ncall missing\n";
    result = bridge.validate_dry_run(script, warnings);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT(warnings.size() == 2);
    return (1);
}
