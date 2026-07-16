#include "../test_internal.hpp"

#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_achievement_thread_safe_lifecycle)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());
    achievement.set_id(42);
    achievement.set_goal(7, 10);
    achievement.set_progress(7, 4);
    FT_ASSERT_EQ(42, achievement.get_id());
    FT_ASSERT_EQ(10, achievement.get_goal(7));
    FT_ASSERT_EQ(4, achievement.get_progress(7));
    FT_ASSERT_EQ(FT_FALSE, achievement.is_goal_complete(7));
    achievement.add_progress(7, 6);
    FT_ASSERT_EQ(FT_TRUE, achievement.is_goal_complete(7));
    FT_ASSERT_EQ(FT_TRUE, achievement.is_complete());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.destroy());
    return (1);
}
