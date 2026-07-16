#include "../test_internal.hpp"

#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_behavior_action_thread_safe_lifecycle)
{
    game_behavior_action action;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.initialize(7, 1.5, 2.25));
    FT_ASSERT_EQ(7, action.get_action_id());
    FT_ASSERT_EQ(1.5, action.get_weight());
    FT_ASSERT_EQ(2.25, action.get_cooldown_seconds());
    action.set_action_id(11);
    action.set_weight(3.75);
    action.set_cooldown_seconds(4.5);
    FT_ASSERT_EQ(11, action.get_action_id());
    FT_ASSERT_EQ(3.75, action.get_weight());
    FT_ASSERT_EQ(4.5, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.destroy());
    return (1);
}
