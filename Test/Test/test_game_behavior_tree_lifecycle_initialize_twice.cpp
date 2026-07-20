#include "../../Modules/Game/game_behavior_tree.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_game_behavior_tree_initialize_twice_reports_invalid_state)
{
    game_behavior_tree value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, value.initialize());
    return (1);
}
