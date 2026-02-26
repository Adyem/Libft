#include "../test_internal.hpp"
#include "../../Game/game_scripting_bridge.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/shared_ptr.hpp"
#include <string>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int g_script_callback_invocations = 0;
static int g_script_last_score = 0;

static int game_script_adjust_score(ft_game_script_context &context, const ft_vector<ft_string> &arguments) noexcept
{
    const ft_string *score_value;
    int base_score;
    int delta;
    ft_string updated_score;

    ft_string score_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, score_key.initialize("score"));
    score_value = context.get_variable(score_key);
    if (score_value == ft_nullptr)
        return (context.get_error());
    base_score = ft_atoi(score_value->c_str());
    if (context.get_error() != FT_ERR_SUCCESS)
        return (context.get_error());
    if (arguments.size() > 0)
        delta = ft_atoi(arguments[0].c_str());
    else
        delta = 0;
    {
        std::string numeric_string = std::to_string(static_cast<long>(base_score + delta));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, updated_score.initialize(numeric_string.c_str()));
    }
    context.set_variable(score_key, updated_score);
    if (context.get_error() != FT_ERR_SUCCESS)
        return (context.get_error());
    g_script_last_score = base_score + delta;
    g_script_callback_invocations += 1;
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_game_script_context_variable_controls,
    "ft_game_script_context stores, updates, and clears script variables")
{
    ft_game_script_context context;
    const ft_string *value_pointer;

    ft_string quest_stage_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_stage_key.initialize("quest_stage"));
    ft_string quest_stage_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_stage_value.initialize("3"));
    context.set_variable(quest_stage_key, quest_stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(quest_stage_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    FT_ASSERT_STR_EQ("3", value_pointer->c_str());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_stage_value.initialize("4"));
    context.set_variable(quest_stage_key, quest_stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(quest_stage_key);
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());
    FT_ASSERT_STR_EQ("4", value_pointer->c_str());

    context.remove_variable(quest_stage_key);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(quest_stage_key);
    FT_ASSERT(value_pointer == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_stage_value.initialize("7"));
    context.set_variable(quest_stage_key, quest_stage_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    context.clear_variables();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.get_error());

    value_pointer = context.get_variable(quest_stage_key);
    FT_ASSERT(value_pointer == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());

    return (1);
}

FT_TEST(test_game_script_bridge_executes_callbacks,
    "ft_game_script_bridge executes registered callbacks and updates variables")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> adjust_function(game_script_adjust_score);
    ft_string script;
    int register_result;
    int execute_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    ft_string adjust_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, adjust_key.initialize("adjust_score"));
    register_result = bridge.register_function(adjust_key, adjust_function);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    g_script_callback_invocations = 0;
    g_script_last_score = 0;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 10\ncall adjust_score 5\ncall adjust_score 3\n"));

    execute_result = bridge.execute(script, state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, execute_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(2, g_script_callback_invocations);
    FT_ASSERT_EQ(18, g_script_last_score);

    return (1);
}

FT_TEST(test_game_script_bridge_operation_limit,
    "ft_game_script_bridge enforces operation limits before executing extra lines")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> adjust_function(game_script_adjust_score);
    ft_string script;
    int register_result;
    int execute_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    ft_string adjust_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, adjust_key.initialize("adjust_score"));
    register_result = bridge.register_function(adjust_key, adjust_function);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    bridge.set_max_operations(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    g_script_callback_invocations = 0;
    g_script_last_score = 0;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 1\ncall adjust_score 1\n"));

    execute_result = bridge.execute(script, state);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, execute_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, bridge.get_error());
    FT_ASSERT_EQ(0, g_script_callback_invocations);
    FT_ASSERT_EQ(0, g_script_last_score);

    return (1);
}

FT_TEST(test_game_script_bridge_sandbox_helper,
    "ft_game_script_bridge reports unsupported commands during sandbox checks")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_vector<ft_string> violations;
    ft_string script;
    int inspection_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 4\nteleport player base\ncall adjust_score 1\n"));

    inspection_result = bridge.check_sandbox_capabilities(script, violations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inspection_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT(violations.size() == 1);
    FT_ASSERT(violations[0] == "unsupported command: teleport");

    return (1);
}

FT_TEST(test_game_script_bridge_dry_run_helper,
    "ft_game_script_bridge dry-run validation collects callback and argument warnings")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_vector<ft_string> warnings;
    ft_string script;
    int validation_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("call missing\nset score\nunset\n"));

    validation_result = bridge.validate_dry_run(script, warnings);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, validation_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT(warnings.size() == 3);
    FT_ASSERT(warnings[0] == "unregistered callback: missing");
    FT_ASSERT(warnings[1] == "set missing value for key: score");
    FT_ASSERT(warnings[2] == "unset missing key");

    return (1);
}

FT_TEST(test_game_script_bridge_bytecode_budget_helper,
    "ft_game_script_bridge inspector reports operation counts before execution")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_script_bridge bridge;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.initialize(world_pointer));
    ft_string script;
    int required_operations;
    int inspection_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize("set score 10\ncall adjust_score 2\nunset score\n"));

    required_operations = 0;
    inspection_result = bridge.inspect_bytecode_budget(script, required_operations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inspection_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(3, required_operations);

    bridge.set_max_operations(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bridge.get_error());

    inspection_result = bridge.inspect_bytecode_budget(script, required_operations);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, inspection_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, bridge.get_error());
    FT_ASSERT_EQ(3, required_operations);

    return (1);
}
