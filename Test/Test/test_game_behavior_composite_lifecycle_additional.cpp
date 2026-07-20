#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void composite_initialize_twice(game_behavior_composite &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_behavior_composite_initialize_twice_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_composite>(
                        composite_initialize_twice));
    return (1);
}

FT_TEST(test_game_behavior_composite_initialize_succeeds)
{
    game_behavior_selector value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_behavior_composite_destroy_twice_is_safe)
{
    game_behavior_selector value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_composite_reinitialize_after_destroy)
{
    game_behavior_selector value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_composite_self_move_is_safe)
{
    game_behavior_selector value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(0, value.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_composite_empty_children_are_clearable)
{
    game_behavior_selector value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.clear_children();
    FT_ASSERT_EQ(0, value.get_children().size());
    return (1);
}
