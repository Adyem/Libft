#include "../test_internal.hpp"

#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_item_modifier_thread_safe_lifecycle)
{
    game_item_modifier modifier;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(4, 12));
    FT_ASSERT_EQ(4, modifier.get_id());
    FT_ASSERT_EQ(12, modifier.get_value());
    modifier.set_id(9);
    modifier.set_value(21);
    FT_ASSERT_EQ(9, modifier.get_id());
    FT_ASSERT_EQ(21, modifier.get_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    return (1);
}
