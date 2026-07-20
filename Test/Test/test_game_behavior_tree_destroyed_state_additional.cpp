#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void tree_destroyed_set_root(game_behavior_tree &value)
{
    ft_sharedptr<game_behavior_node> root;

    value.set_root(root);
    return ;
}

static void tree_destroyed_get_root(game_behavior_tree &value)
{
    (void)value.get_root();
    return ;
}

static void tree_destroyed_get_root_const(game_behavior_tree &value)
{
    const game_behavior_tree &constant_value = value;

    (void)constant_value.get_root();
    return ;
}

static void tree_destroyed_tick(game_behavior_tree &value)
{
    game_behavior_context context;

    (void)value.tick(context);
    return ;
}

FT_TEST(test_game_behavior_tree_destroyed_set_root_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_tree>(
                        tree_destroyed_set_root));
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_get_root_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_tree>(
                        tree_destroyed_get_root));
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_get_root_const_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_tree>(
                        tree_destroyed_get_root_const));
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_tick_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_behavior_tree>(
                        tree_destroyed_tick));
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_get_error_is_valid)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_get_error_str_is_valid)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_destructor_is_non_aborting)
{
    game_behavior_tree *value;

    value = new game_behavior_tree();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_destroy_is_idempotent)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_state_can_reinitialize)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_behavior_tree_destroyed_self_move_is_safe)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}
