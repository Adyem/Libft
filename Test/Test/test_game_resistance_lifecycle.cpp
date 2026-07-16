#include "../test_internal.hpp"

#include "../../Modules/Game/game_resistance.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_resistance_thread_safe_lifecycle)
{
    game_resistance resistance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.initialize());
    resistance.set_values(15, 8);
    FT_ASSERT_EQ(15, resistance.get_percent());
    FT_ASSERT_EQ(8, resistance.get_flat());
    resistance.set_percent(3);
    resistance.set_flat(1);
    FT_ASSERT_EQ(3, resistance.get_percent());
    FT_ASSERT_EQ(1, resistance.get_flat());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.reset());
    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.destroy());
    return (1);
}
