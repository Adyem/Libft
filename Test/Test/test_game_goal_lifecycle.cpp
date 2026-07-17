#include "../test_internal.hpp"

#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_goal_thread_safe_lifecycle)
{
    game_goal goal;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.initialize());
    FT_ASSERT_EQ(0, goal.get_target());
    FT_ASSERT_EQ(0, goal.get_progress());
    goal.set_target(5);
    goal.set_progress(3);
    FT_ASSERT_EQ(5, goal.get_target());
    FT_ASSERT_EQ(3, goal.get_progress());
    FT_ASSERT_EQ(FT_FALSE, static_cast<ft_bool>(goal.get_progress()
        >= goal.get_target()));
    goal.add_progress(2);
    FT_ASSERT_EQ(FT_TRUE, static_cast<ft_bool>(goal.get_progress()
        >= goal.get_target()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, goal.destroy());
    return (1);
}
