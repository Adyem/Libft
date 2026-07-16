#include "../test_internal.hpp"

#include "../../Modules/Game/game_equipment.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_equipment_thread_safe_lifecycle)
{
    game_equipment equipment;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.initialize());
    FT_ASSERT(equipment.get_item(EQUIP_HEAD) != ft_nullptr);
    FT_ASSERT(equipment.get_item(EQUIP_CHEST) != ft_nullptr);
    FT_ASSERT(equipment.get_item(EQUIP_WEAPON) != ft_nullptr);
    FT_ASSERT(equipment.get_item(EQUIP_HEAD)->get() == ft_nullptr);
    FT_ASSERT(equipment.get_item(EQUIP_CHEST)->get() == ft_nullptr);
    FT_ASSERT(equipment.get_item(EQUIP_WEAPON)->get() == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.enable_thread_safety());
    FT_ASSERT(equipment.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.destroy());
    return (1);
}
