#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_behavior_composite_add_child(game_behavior_composite &value)
{
    ft_sharedptr<game_behavior_node> child;

    value.add_child(child);
    return ;
}

static void
game_behavior_composite_clear_children(game_behavior_composite &value)
{
    value.clear_children();
    return ;
}

static void game_behavior_composite_get_children(game_behavior_composite &value)
{
    (void)value.get_children();
    return ;
}

static void
game_behavior_composite_get_children_const(game_behavior_composite &value)
{
    const game_behavior_composite &constant_value = value;

    (void)constant_value.get_children();
    return ;
}

static void game_behavior_composite_get_error(game_behavior_composite &value)
{
    (void)value.get_error();
    return ;
}

static void
game_behavior_composite_get_error_str(game_behavior_composite &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_behavior_composite_add_child_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_add_child));
    return (1);
}

FT_TEST(test_game_behavior_composite_clear_children_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_clear_children));
    return (1);
}

FT_TEST(test_game_behavior_composite_get_children_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_get_children));
    return (1);
}

FT_TEST(test_game_behavior_composite_get_children_const_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_get_children_const));
    return (1);
}

FT_TEST(test_game_behavior_composite_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_get_error));
    return (1);
}

FT_TEST(test_game_behavior_composite_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        game_behavior_composite_get_error_str));
    return (1);
}
