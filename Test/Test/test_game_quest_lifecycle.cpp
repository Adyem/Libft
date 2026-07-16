#include "../test_internal.hpp"

#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_quest_thread_safe_lifecycle)
{
    game_quest quest;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_id(19);
    quest.set_phases(4);
    quest.set_current_phase(1);
    quest.set_reward_experience(250);
    FT_ASSERT_EQ(19, quest.get_id());
    FT_ASSERT_EQ(4, quest.get_phases());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(250, quest.get_reward_experience());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.destroy());
    return (1);
}
