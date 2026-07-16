#include "../test_internal.hpp"

#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_item_thread_safe_lifecycle)
{
    game_item item;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    FT_ASSERT_EQ(0, item.get_max_stack());
    FT_ASSERT_EQ(0, item.get_stack_size());
    FT_ASSERT_EQ(0, item.get_item_id());
    item.set_max_stack(64);
    item.set_stack_size(16);
    item.set_item_id(101);
    item.set_width(2);
    item.set_height(3);
    item.set_rarity(4);
    FT_ASSERT_EQ(64, item.get_max_stack());
    FT_ASSERT_EQ(16, item.get_stack_size());
    FT_ASSERT_EQ(101, item.get_item_id());
    FT_ASSERT_EQ(2, item.get_width());
    FT_ASSERT_EQ(3, item.get_height());
    FT_ASSERT_EQ(4, item.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}
