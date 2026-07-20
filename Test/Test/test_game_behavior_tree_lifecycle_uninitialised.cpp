#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_behavior_tree_set_root(game_behavior_tree &value)
{
    ft_sharedptr<game_behavior_node> root;

    value.set_root(root);
    return ;
}

static void game_behavior_tree_get_root(game_behavior_tree &value)
{
    (void)value.get_root();
    return ;
}

static void game_behavior_tree_get_root_const(game_behavior_tree &value)
{
    const game_behavior_tree &constant_value = value;

    (void)constant_value.get_root();
    return ;
}

static void game_behavior_tree_tick(game_behavior_tree &value)
{
    game_behavior_context context;

    (void)value.tick(context);
    return ;
}

static void game_behavior_tree_get_error(game_behavior_tree &value)
{
    (void)value.get_error();
    return ;
}

static void game_behavior_tree_get_error_str(game_behavior_tree &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_behavior_tree_set_root_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_set_root));
    return (1);
}

FT_TEST(test_game_behavior_tree_get_root_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_get_root));
    return (1);
}

FT_TEST(test_game_behavior_tree_get_root_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_get_root_const));
    return (1);
}

FT_TEST(test_game_behavior_tree_tick_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_tick));
    return (1);
}

FT_TEST(test_game_behavior_tree_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_get_error));
    return (1);
}

FT_TEST(test_game_behavior_tree_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_tree>(
                        game_behavior_tree_get_error_str));
    return (1);
}
