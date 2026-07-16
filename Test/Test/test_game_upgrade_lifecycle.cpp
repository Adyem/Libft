#include "../test_internal.hpp"

#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_upgrade_thread_safe_lifecycle)
{
    game_upgrade upgrade;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.initialize());
    upgrade.set_id(27);
    upgrade.set_current_level(2);
    upgrade.set_max_level(9);
    upgrade.set_modifier1(5);
    upgrade.set_modifier2(6);
    upgrade.set_modifier3(7);
    upgrade.set_modifier4(8);
    FT_ASSERT_EQ(27, upgrade.get_id());
    FT_ASSERT_EQ(2, upgrade.get_current_level());
    FT_ASSERT_EQ(9, upgrade.get_max_level());
    FT_ASSERT_EQ(5, upgrade.get_modifier1());
    FT_ASSERT_EQ(6, upgrade.get_modifier2());
    FT_ASSERT_EQ(7, upgrade.get_modifier3());
    FT_ASSERT_EQ(8, upgrade.get_modifier4());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.destroy());
    return (1);
}
