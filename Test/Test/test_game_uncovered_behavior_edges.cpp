#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Game/game_behavior_tree.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_behavior_edge_context_null_defaults)
{
    game_behavior_context context;
    FT_ASSERT(context.get_character() == ft_nullptr);
    FT_ASSERT(context.get_user_data() == ft_nullptr);
    return (1);
}

FT_TEST(test_game_behavior_edge_context_round_trip)
{
    game_behavior_context context;
    void *user_data;
    user_data = reinterpret_cast<void *>(static_cast<uintptr_t>(17));
    context.set_user_data(user_data);
    FT_ASSERT(context.get_user_data() == user_data);
    return (1);
}

FT_TEST(test_game_behavior_edge_context_copy_null)
{
    game_behavior_context source;
    game_behavior_context copy(source);
    FT_ASSERT(copy.get_character() == ft_nullptr);
    FT_ASSERT(copy.get_user_data() == ft_nullptr);
    return (1);
}

FT_TEST(test_game_behavior_edge_action_callback_round_trip)
{
    game_behavior_tree_action action;
    ft_function<int32_t(game_behavior_context &)> callback(
        [](game_behavior_context &context)
        {
            (void)context;
            return (FT_BEHAVIOR_STATUS_SUCCESS);
        });
    action.set_callback(callback);
    FT_ASSERT(static_cast<ft_bool>(action.get_callback()));
    return (1);
}

FT_TEST(test_game_behavior_edge_action_success_callback)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_SUCCESS);
        }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_action_failure_callback)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_FAILURE);
        }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_action_running_callback)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_RUNNING);
        }));
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_action_invalid_callback_status)
{
    game_behavior_tree_action action;
    game_behavior_context context;
    action.set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (99);
        }));
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, action.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_selector_empty_failure)
{
    game_behavior_selector selector;
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_sequence_empty_success)
{
    game_behavior_sequence sequence;
    game_behavior_context context;
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_selector_clear_children)
{
    game_behavior_selector selector;
    selector.clear_children();
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), selector.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_edge_sequence_clear_children)
{
    game_behavior_sequence sequence;
    sequence.clear_children();
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), sequence.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_edge_tree_default_root)
{
    game_behavior_tree tree;
    FT_ASSERT(tree.get_root().get() == ft_nullptr);
    return (1);
}

FT_TEST(test_game_behavior_edge_tree_tick_without_root)
{
    game_behavior_tree tree;
    game_behavior_context context;
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, tree.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_tree_action_root)
{
    game_behavior_tree tree;
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> root(new game_behavior_tree_action());
    static_cast<game_behavior_tree_action *>(root.get())->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_SUCCESS);
        }));
    tree.set_root(root);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, tree.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_selector_first_success)
{
    game_behavior_selector selector;
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child(new game_behavior_tree_action());
    static_cast<game_behavior_tree_action *>(child.get())->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_SUCCESS);
        }));
    selector.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_sequence_first_failure)
{
    game_behavior_sequence sequence;
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child(new game_behavior_tree_action());
    static_cast<game_behavior_tree_action *>(child.get())->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_FAILURE);
        }));
    sequence.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_selector_running)
{
    game_behavior_selector selector;
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child(new game_behavior_tree_action());
    static_cast<game_behavior_tree_action *>(child.get())->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_RUNNING);
        }));
    selector.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, selector.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_sequence_running)
{
    game_behavior_sequence sequence;
    game_behavior_context context;
    ft_sharedptr<game_behavior_node> child(new game_behavior_tree_action());
    static_cast<game_behavior_tree_action *>(child.get())->set_callback(ft_function<int32_t(game_behavior_context &)>(
        [](game_behavior_context &value)
        {
            (void)value;
            return (FT_BEHAVIOR_STATUS_RUNNING);
        }));
    sequence.add_child(child);
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_RUNNING, sequence.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_edge_composite_child_count)
{
    game_behavior_selector selector;
    ft_sharedptr<game_behavior_node> first(new game_behavior_tree_action());
    ft_sharedptr<game_behavior_node> second(new game_behavior_tree_action());
    selector.add_child(first);
    selector.add_child(second);
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), selector.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_edge_tree_root_replacement)
{
    game_behavior_tree tree;
    ft_sharedptr<game_behavior_node> first(new game_behavior_tree_action());
    ft_sharedptr<game_behavior_node> second(new game_behavior_tree_action());
    tree.set_root(first);
    tree.set_root(second);
    FT_ASSERT(tree.get_root().get() == second.get());
    return (1);
}
