#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void sequence_initialize_twice(game_behavior_sequence &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_behavior_sequence_initialize_twice_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_behavior_sequence>(
                        sequence_initialize_twice));
    return (1);
}

FT_TEST(test_game_behavior_sequence_initialize_succeeds)
{
    game_behavior_sequence value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_behavior_sequence_destroy_twice_is_safe)
{
    game_behavior_sequence value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_sequence_reinitialize_after_destroy)
{
    game_behavior_sequence value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_sequence_self_move_is_safe)
{
    game_behavior_sequence value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(0, value.get_children().size());
    return (1);
}

FT_TEST(test_game_behavior_sequence_empty_tick_reports_success)
{
    game_behavior_sequence value;
    game_behavior_context context;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_SUCCESS, value.tick(context));
    return (1);
}
