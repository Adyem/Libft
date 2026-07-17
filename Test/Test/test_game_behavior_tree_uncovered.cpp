#include "../test_internal.hpp"
#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Template/function.hpp"
#include "../../Modules/Template/shared_ptr.hpp"

static void make_behavior_action(ft_sharedptr<game_behavior_node> &action,
    int32_t status)
{
    game_behavior_tree_action *action_pointer;

    action_pointer = new game_behavior_tree_action();
    action.initialize(action_pointer);
    action_pointer->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [status](game_behavior_context &) -> int32_t
        {
            return (status);
        }));
    return ;
}

FT_TEST(test_game_behavior_context_defaults)
{
    game_behavior_context context;
    FT_ASSERT_EQ(ft_nullptr, context.get_character());
    FT_ASSERT_EQ(ft_nullptr, context.get_user_data());
    return (1);
}

FT_TEST(test_game_behavior_context_user_data)
{
    game_behavior_context context;
    void *user_data = reinterpret_cast<void *>(static_cast<uintptr_t>(2U));
    context.set_user_data(user_data);
    FT_ASSERT_EQ(user_data, context.get_user_data());
    return (1);
}

FT_TEST(test_game_behavior_context_copy)
{
    game_behavior_context source;
    void *user_data = reinterpret_cast<void *>(static_cast<uintptr_t>(3U));
    source.set_user_data(user_data);
    game_behavior_context copy(source);
    FT_ASSERT_EQ(user_data, copy.get_user_data());
    return (1);
}

FT_TEST(test_game_behavior_context_move_clears_source)
{
    game_behavior_context source;
    void *user_data = reinterpret_cast<void *>(static_cast<uintptr_t>(4U));
    source.set_user_data(user_data);
    game_behavior_context moved(static_cast<game_behavior_context &&>(source));
    FT_ASSERT_EQ(user_data, moved.get_user_data());
    FT_ASSERT_EQ(ft_nullptr, source.get_user_data());
    return (1);
}

FT_TEST(test_game_behavior_action_without_callback)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, action.tick(context));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, action.get_error());
    return (1);
}

FT_TEST(test_game_behavior_action_running)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &) -> int32_t { return (FT_BEHAVIOR_STATUS_RUNNING); }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_action_success)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &) -> int32_t { return (FT_BEHAVIOR_STATUS_SUCCESS); }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, action.tick(context));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_game_behavior_action_failure)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &) -> int32_t { return (FT_BEHAVIOR_STATUS_FAILURE); }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_action_invalid_status)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &) -> int32_t { return (99); }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, action.tick(context));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, action.get_error());
    return (1);
}

FT_TEST(test_game_behavior_action_callback_getter)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &) -> int32_t { return (FT_BEHAVIOR_STATUS_SUCCESS); }));
    FT_ASSERT(static_cast<ft_bool>(action.get_callback()));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, action.get_callback()(context));
    return (1);
}

FT_TEST(test_game_behavior_selector_empty)
{
    game_behavior_selector selector;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, selector.initialize());
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_empty)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_composite_null_child)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    ft_sharedptr<game_behavior_node> child;
    sequence.add_child(child);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, sequence.get_error());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), sequence.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_composite_add_and_clear)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    ft_sharedptr<game_behavior_node> child;
    make_behavior_action(child, FT_BEHAVIOR_STATUS_SUCCESS);
    sequence.add_child(child);
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), sequence.get_children().size());
    sequence.clear_children();
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), sequence.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_selector_success)
{
    game_behavior_selector selector;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, selector.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child;
    make_behavior_action(child, FT_BEHAVIOR_STATUS_SUCCESS);
    selector.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_selector_failure_then_success)
{
    game_behavior_selector selector;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, selector.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_FAILURE);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_SUCCESS);
    selector.add_child(first);
    selector.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_selector_running_stops)
{
    game_behavior_selector selector;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, selector.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_RUNNING);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_SUCCESS);
    selector.add_child(first);
    selector.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_selector_all_failure)
{
    game_behavior_selector selector;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, selector.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_FAILURE);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_FAILURE);
    selector.add_child(first);
    selector.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_success)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child;
    make_behavior_action(child, FT_BEHAVIOR_STATUS_SUCCESS);
    sequence.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_failure_stops)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_FAILURE);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_SUCCESS);
    sequence.add_child(first);
    sequence.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_running_stops)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_RUNNING);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_FAILURE);
    sequence.add_child(first);
    sequence.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_all_success)
{
    game_behavior_sequence sequence;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> first;
    ft_sharedptr<game_behavior_node> second;
    make_behavior_action(first, FT_BEHAVIOR_STATUS_SUCCESS);
    make_behavior_action(second, FT_BEHAVIOR_STATUS_SUCCESS);
    sequence.add_child(first);
    sequence.add_child(second);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_tree_without_root)
{
    game_behavior_tree tree;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, tree.tick(context));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, tree.get_error());
    return (1);
}

FT_TEST(test_game_behavior_tree_root_round_trip)
{
    game_behavior_tree tree;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    ft_sharedptr<game_behavior_node> root;
    make_behavior_action(root, FT_BEHAVIOR_STATUS_SUCCESS);
    tree.set_root(root);
    FT_ASSERT(tree.get_root());
    return (1);
}

FT_TEST(test_game_behavior_tree_success)
{
    game_behavior_tree tree;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> root;
    make_behavior_action(root, FT_BEHAVIOR_STATUS_SUCCESS);
    tree.set_root(root);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, tree.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_tree_running)
{
    game_behavior_tree tree;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> root;
    make_behavior_action(root, FT_BEHAVIOR_STATUS_RUNNING);
    tree.set_root(root);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, tree.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_tree_failure)
{
    game_behavior_tree tree;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> root;
    make_behavior_action(root, FT_BEHAVIOR_STATUS_FAILURE);
    tree.set_root(root);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, tree.tick(context));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.get_error());
    return (1);
}

FT_TEST(test_game_behavior_tree_lifecycle)
{
    game_behavior_tree tree;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, tree.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tree.destroy());
    return (1);
}

FT_TEST(test_game_behavior_composite_requires_initialization)
{
    game_behavior_sequence sequence;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sequence.destroy());
    sequence.clear_children();
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, sequence.get_error());
    return (1);
}
