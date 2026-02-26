#include "../test_internal.hpp"
#include "../../Game/game_scripting_bridge.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../Template/shared_ptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_script_context_default_constructor_sets_success,
    "Game: script context default constructor sets success state")
{
    ft_game_script_context context;

    FT_ASSERT_EQ(ft_nullptr, context.get_state());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    FT_ASSERT_EQ(ft_nullptr, context.get_world().get());
    return (1);
}

FT_TEST(test_game_script_context_initialize_stores_inputs,
    "Game: script context initialize stores state and world")
{
    ft_game_state state;
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_context context;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize(&state, world_pointer));
    FT_ASSERT_EQ(&state, context.get_state());
    FT_ASSERT_EQ(world_pointer.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_initialize_copy_preserves_error,
    "Game: script context initialize(copy) copies data and error")
{
    ft_game_script_context original;
    ft_game_script_context copy;
    const ft_string *value_pointer;

    ft_string missing_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, missing_key.initialize("missing"));
    value_pointer = original.get_variable(missing_key);
    FT_ASSERT_EQ(ft_nullptr, value_pointer);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, original.get_error());

    ft_string quest_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_key.initialize("quest"));
    ft_string quest_val;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_val.initialize("alpha"));
    original.set_variable(quest_key, quest_val);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(original));

    FT_ASSERT_EQ(original.get_state(), copy.get_state());
    FT_ASSERT_EQ(original.get_world().get(), copy.get_world().get());
    value_pointer = copy.get_variable(quest_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_STR_EQ("alpha", value_pointer->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    return (1);
}

FT_TEST(test_game_script_context_initialize_copy_replaces_state,
    "Game: script context initialize(copy) replaces stored state and variables")
{
    ft_game_state first_state;
    ft_game_state second_state;
    ft_game_script_context source;
    ft_game_script_context destination;
    const ft_string *value_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        source.initialize(&second_state, ft_sharedptr<ft_world>(new ft_world())));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination.initialize(&first_state, ft_sharedptr<ft_world>(new ft_world())));
    ft_string score_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_key.initialize("score"));
    ft_string score_val;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_val.initialize("42"));
    source.set_variable(score_key, score_val);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_val.initialize("7"));
    destination.set_variable(score_key, score_val);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(&second_state, destination.get_state());
    FT_ASSERT_EQ(source.get_world().get(), destination.get_world().get());

    value_pointer = destination.get_variable(score_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_STR_EQ("42", value_pointer->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_state_clears_previous_error,
    "Game: set_state updates pointer and resets error")
{
    ft_game_state initial_state;
    ft_game_state updated_state;
    ft_game_script_context context;
    const ft_string *value_pointer;

    ft_string unknown_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unknown_key.initialize("unknown"));
    value_pointer = context.get_variable(unknown_key);
    FT_ASSERT_EQ(ft_nullptr, value_pointer);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());

    context.set_state(&initial_state);
    FT_ASSERT_EQ(&initial_state, context.get_state());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.set_state(&updated_state);
    FT_ASSERT_EQ(&updated_state, context.get_state());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_world_updates_pointer,
    "Game: set_world stores shared pointer and resets error")
{
    ft_game_script_context context;
    ft_sharedptr<ft_world> first_world(new ft_world());
    ft_sharedptr<ft_world> second_world(new ft_world());

    context.set_world(first_world);
    FT_ASSERT_EQ(first_world.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.set_world(second_world);
    FT_ASSERT_EQ(second_world.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_variable_overwrites_existing,
    "Game: set_variable overwrites existing entries and keeps success")
{
    ft_game_script_context context;
    const ft_string *value_pointer;

    ft_string stage_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_key.initialize("stage"));
    ft_string stage_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_value.initialize("1"));
    context.set_variable(stage_key, stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_value.initialize("2"));
    context.set_variable(stage_key, stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(stage_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_STR_EQ("2", value_pointer->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_remove_variable_clears_entry,
    "Game: remove_variable deletes entry and reports not found after removal")
{
    ft_game_script_context context;
    const ft_string *value_pointer;

    ft_string target_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_key.initialize("target"));
    ft_string target_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_value.initialize("enabled"));
    context.set_variable(target_key, target_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.remove_variable(target_key);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(target_key);
    FT_ASSERT_EQ(ft_nullptr, value_pointer);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_clear_variables_removes_all,
    "Game: clear_variables removes stored entries and resets error")
{
    ft_game_script_context context;
    const ft_string *value_pointer;

    ft_string alpha_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, alpha_key.initialize("alpha"));
    ft_string alpha_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, alpha_value.initialize("a"));
    ft_string beta_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, beta_key.initialize("beta"));
    ft_string beta_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, beta_value.initialize("b"));
    context.set_variable(alpha_key, alpha_value);
    context.set_variable(beta_key, beta_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.clear_variables();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(alpha_key);
    FT_ASSERT_EQ(ft_nullptr, value_pointer);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_get_error_str_matches_errno,
    "Game: get_error_str matches ft_strerror output")
{
    ft_game_script_context context;
    const char *message;

    ft_string key_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key_name.initialize("key"));
    ft_string key_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key_value.initialize("value"));
    context.set_variable(key_name, key_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    message = context.get_error_str();
    FT_ASSERT(message != ft_nullptr);
    FT_ASSERT(ft_strncmp(message, ft_strerror(FT_ERR_SUCCESS), 32) == 0);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_counts_supported_commands,
    "Game: inspect_bytecode_budget counts supported commands")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 1\ncall adjust\nunset score\n"));
    int operations;
    int result;

    bridge.set_max_operations(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(3, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_rejects_unknown_command,
    "Game: inspect_bytecode_budget rejects unsupported commands")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("jump now\n"));
    int operations;
    int result;

    operations = -1;
    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_EQ(0, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_enforces_max_operations,
    "Game: inspect_bytecode_budget enforces max operation budget")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 1\ncall adjust 2\n"));
    int operations;
    int result;

    bridge.set_max_operations(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, bridge.get_error());
    FT_ASSERT_EQ(2, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_ignores_comments_and_blank_lines,
    "Game: inspect_bytecode_budget ignores comments and empty lines")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("-- comment\n\n# ignored\nset gold 5\n"));
    int operations;
    int result;

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(1, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_handles_crlf_lines,
    "Game: inspect_bytecode_budget counts commands with mixed line endings")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set a 1\r\n\r\nunset a\r\n"));
    int operations;
    int result;

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(2, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_handles_empty_script,
    "Game: inspect_bytecode_budget returns zero for empty scripts")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    int operations;
    int result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize(""));

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(0, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_resets_errno_on_success,
    "Game: inspect_bytecode_budget clears errno on success")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set timer 1\n"));
    int operations;
    int result;

    result = bridge.inspect_bytecode_budget(script, operations);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(1, operations);
    return (1);
}

FT_TEST(test_game_script_bridge_inspect_budget_reuses_previous_limit_after_invalid_update,
    "Game: inspect_bytecode_budget uses existing limit when update fails")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set a 1\ncall go\n"));
    int operations;
    int result;

    bridge.set_max_operations(-2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, bridge.get_error());
    FT_ASSERT_EQ(32, bridge.get_max_operations());

    result = bridge.inspect_bytecode_budget(script, operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(2, operations);
    return (1);
}
