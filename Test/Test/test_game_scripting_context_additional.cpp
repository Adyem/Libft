#include "../test_internal.hpp"
#include "../../Modules/Game/game_scripting_bridge.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Template/shared_ptr.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_script_context_default_constructor_sets_success)
{
    game_script_context context;

    FT_ASSERT_EQ(FT_CLASS_STATE_UNINITIALISED, context._initialised_state);
    FT_ASSERT_EQ(ft_nullptr, context.get_state());
    FT_ASSERT_EQ(ft_nullptr, context.get_world().get());
    return (1);
}

FT_TEST(test_game_script_context_initialize_stores_inputs)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_context context;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize(&state, world_pointer));
    FT_ASSERT_EQ(&state, context.get_state());
    FT_ASSERT_EQ(world_pointer.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_initialize_copy_preserves_error)
{
    game_script_context original;
    game_script_context copy;
    const ft_string *value_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());

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

FT_TEST(test_game_script_context_initialize_copy_replaces_state)
{
    game_state first_state;
    game_state second_state;
    game_script_context source;
    game_script_context destination;
    const ft_string *value_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_state.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_state.initialize());

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        source.initialize(&second_state, ft_sharedptr<game_world>(new game_world())));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination.initialize(&first_state, ft_sharedptr<game_world>(new game_world())));
    ft_string score_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_key.initialize("score"));
    ft_string score_val;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_val.initialize("42"));
    source.set_variable(score_key, score_val);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_val.clear());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_val.append("7"));
    destination.set_variable(score_key, score_val);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(&second_state, destination.get_state());
    FT_ASSERT_EQ(source.get_world().get(), destination.get_world().get());

    value_pointer = destination.get_variable(score_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_STR_EQ("42", value_pointer->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_state_clears_previous_error)
{
    game_state initial_state;
    game_state updated_state;
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, initial_state.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, updated_state.initialize());
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

FT_TEST(test_game_script_context_set_world_updates_pointer)
{
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
    ft_sharedptr<game_world> first_world(new game_world());
    FT_ASSERT(first_world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_world->initialize());
    ft_sharedptr<game_world> second_world(new game_world());

    context.set_world(first_world);
    FT_ASSERT_EQ(first_world.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.set_world(second_world);
    FT_ASSERT_EQ(second_world.get(), context.get_world().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_set_variable_overwrites_existing)
{
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
    const ft_string *value_pointer;

    ft_string stage_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_key.initialize("stage"));
    ft_string stage_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_value.initialize("1"));
    context.set_variable(stage_key, stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_value.clear());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stage_value.append("2"));
    context.set_variable(stage_key, stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(stage_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_STR_EQ("2", value_pointer->c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    return (1);
}

FT_TEST(test_game_script_context_remove_variable_clears_entry)
{
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
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

FT_TEST(test_game_script_context_clear_variables_removes_all)
{
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
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

FT_TEST(test_game_script_context_get_error_str_matches_errno)
{
    game_script_context context;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize());
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

FT_TEST(test_game_script_bridge_inspect_budget_counts_supported_commands)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_rejects_unknown_command)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_enforces_max_operations)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_ignores_comments_and_blank_lines)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_handles_crlf_lines)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_handles_empty_script)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_resets_errno_on_success)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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

FT_TEST(test_game_script_bridge_inspect_budget_reuses_previous_limit_after_invalid_update)
{
    ft_sharedptr<game_world> world_pointer(new game_world());
    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
    game_script_bridge bridge;
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
