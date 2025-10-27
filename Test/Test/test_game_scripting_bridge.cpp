#include "../../Game/game_scripting_bridge.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/shared_ptr.hpp"

static int g_script_callback_invocations = 0;
static int g_script_last_score = 0;

static int game_script_adjust_score(ft_game_script_context &context, const ft_vector<ft_string> &arguments) noexcept
{
    const ft_string *score_value;
    int base_score;
    int delta;
    ft_string updated_score;

    score_value = context.get_variable("score");
    if (score_value == ft_nullptr)
        return (context.get_error());
    base_score = ft_atoi(score_value->c_str());
    if (context.get_error() != ER_SUCCESS)
        return (context.get_error());
    if (arguments.size() > 0)
        delta = ft_atoi(arguments[0].c_str());
    else
        delta = 0;
    updated_score = ft_to_string(static_cast<long>(base_score + delta));
    if (updated_score.get_error() != ER_SUCCESS)
        return (updated_score.get_error());
    context.set_variable("score", updated_score);
    if (context.get_error() != ER_SUCCESS)
        return (context.get_error());
    g_script_last_score = base_score + delta;
    g_script_callback_invocations += 1;
    return (ER_SUCCESS);
}

FT_TEST(test_game_script_context_variable_controls,
    "ft_game_script_context stores, updates, and clears script variables")
{
    ft_game_script_context context;
    const ft_string *value_pointer;

    context.set_variable("quest_stage", "3");
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());

    value_pointer = context.get_variable("quest_stage");
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());
    FT_ASSERT(*value_pointer == "3");

    context.set_variable("quest_stage", "4");
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());

    value_pointer = context.get_variable("quest_stage");
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());
    FT_ASSERT(*value_pointer == "4");

    context.remove_variable("quest_stage");
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());

    value_pointer = context.get_variable("quest_stage");
    FT_ASSERT(value_pointer == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());

    context.set_variable("quest_stage", "7");
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());

    context.clear_variables();
    FT_ASSERT_EQ(ER_SUCCESS, context.get_error());

    value_pointer = context.get_variable("quest_stage");
    FT_ASSERT(value_pointer == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, context.get_error());

    return (1);
}

FT_TEST(test_game_script_bridge_executes_callbacks,
    "ft_game_script_bridge executes registered callbacks and updates variables")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge(world_pointer);
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> adjust_function(game_script_adjust_score);
    ft_string script;
    int register_result;
    int execute_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, world_pointer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());

    register_result = bridge.register_function("adjust_score", adjust_function);
    FT_ASSERT_EQ(ER_SUCCESS, register_result);
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());

    g_script_callback_invocations = 0;
    g_script_last_score = 0;

    script = "set score 10\ncall adjust_score 5\ncall adjust_score 3\n";
    FT_ASSERT_EQ(ER_SUCCESS, script.get_error());

    execute_result = bridge.execute(script, state);
    FT_ASSERT_EQ(ER_SUCCESS, execute_result);
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());
    FT_ASSERT_EQ(2, g_script_callback_invocations);
    FT_ASSERT_EQ(18, g_script_last_score);

    return (1);
}

FT_TEST(test_game_script_bridge_operation_limit,
    "ft_game_script_bridge enforces operation limits before executing extra lines")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_state state;
    ft_game_script_bridge bridge(world_pointer);
    ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> adjust_function(game_script_adjust_score);
    ft_string script;
    int register_result;
    int execute_result;

    FT_ASSERT(world_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, world_pointer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());

    register_result = bridge.register_function("adjust_score", adjust_function);
    FT_ASSERT_EQ(ER_SUCCESS, register_result);
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());

    bridge.set_max_operations(1);
    FT_ASSERT_EQ(ER_SUCCESS, bridge.get_error());

    g_script_callback_invocations = 0;
    g_script_last_score = 0;

    script = "set score 1\ncall adjust_score 1\n";
    FT_ASSERT_EQ(ER_SUCCESS, script.get_error());

    execute_result = bridge.execute(script, state);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, execute_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, bridge.get_error());
    FT_ASSERT_EQ(0, g_script_callback_invocations);
    FT_ASSERT_EQ(0, g_script_last_score);

    return (1);
}
