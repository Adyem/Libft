#include "../test_internal.hpp"

#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_skill_thread_safe_lifecycle)
{
    game_skill skill;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.initialize());
    skill.set_id(17);
    skill.set_level(6);
    skill.set_cooldown(13);
    skill.set_modifier1(1);
    skill.set_modifier2(2);
    skill.set_modifier3(3);
    skill.set_modifier4(4);
    FT_ASSERT_EQ(17, skill.get_id());
    FT_ASSERT_EQ(6, skill.get_level());
    FT_ASSERT_EQ(13, skill.get_cooldown());
    FT_ASSERT_EQ(1, skill.get_modifier1());
    FT_ASSERT_EQ(2, skill.get_modifier2());
    FT_ASSERT_EQ(3, skill.get_modifier3());
    FT_ASSERT_EQ(4, skill.get_modifier4());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.destroy());
    return (1);
}
