#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

FT_TEST(test_game_behavior_selector_tick_uninitialised_is_safe)
{
    game_behavior_selector value;
    game_behavior_context context;

    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, value.tick(context));
    return (1);
}

FT_TEST(test_game_behavior_sequence_tick_uninitialised_is_safe)
{
    game_behavior_sequence value;
    game_behavior_context context;

    FT_ASSERT_EQ(FT_BEHAVIOR_STATUS_FAILURE, value.tick(context));
    return (1);
}
