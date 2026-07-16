#include "../test_internal.hpp"

#include "../../Modules/Game/game_inventory.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_inventory_thread_safe_lifecycle)
{
    game_inventory inventory;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(16, 128));
    FT_ASSERT_EQ(static_cast<ft_size_t>(16), inventory.get_capacity());
    FT_ASSERT_EQ(0, inventory.get_used());
    FT_ASSERT_EQ(128, inventory.get_weight_limit());
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT(inventory.is_full() == FT_FALSE);
    inventory.set_used_slots(4);
    inventory.set_current_weight(32);
    inventory.set_weight_limit(64);
    FT_ASSERT_EQ(static_cast<ft_size_t>(4), inventory.get_used());
    FT_ASSERT_EQ(32, inventory.get_current_weight());
    FT_ASSERT_EQ(64, inventory.get_weight_limit());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.enable_thread_safety());
    FT_ASSERT(inventory.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.destroy());
    return (1);
}
