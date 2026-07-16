#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Game/game_inventory.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/Template/shared_ptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_item_edge_modifier_parameterized)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(4, -9));
    FT_ASSERT_EQ(4, modifier.get_id());
    FT_ASSERT_EQ(-9, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_edge_modifier_reset_by_reinitialize)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(1, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(0, modifier.get_id());
    FT_ASSERT_EQ(0, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_edge_modifier_copy)
{
    game_item_modifier source;
    game_item_modifier destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(8, 13));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(8, destination.get_id());
    FT_ASSERT_EQ(13, destination.get_value());
    return (1);
}

FT_TEST(test_game_item_edge_modifier_move)
{
    game_item_modifier source;
    game_item_modifier destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(5, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_item_modifier &&>(source)));
    FT_ASSERT_EQ(5, destination.get_id());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_item_edge_stack_add_and_subtract)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    item.set_max_stack(10);
    item.set_stack_size(3);
    item.add_to_stack(5);
    item.sub_from_stack(2);
    FT_ASSERT_EQ(6, item.get_stack_size());
    return (1);
}

FT_TEST(test_game_item_edge_dimensions)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    item.set_width(0);
    item.set_height(9);
    FT_ASSERT_EQ(0, item.get_width());
    FT_ASSERT_EQ(9, item.get_height());
    return (1);
}

FT_TEST(test_game_item_edge_all_modifier_slots)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    item.set_modifier1_id(1);
    item.set_modifier2_id(2);
    item.set_modifier3_id(3);
    item.set_modifier4_id(4);
    FT_ASSERT_EQ(1, item.get_modifier1_id());
    FT_ASSERT_EQ(2, item.get_modifier2_id());
    FT_ASSERT_EQ(3, item.get_modifier3_id());
    FT_ASSERT_EQ(4, item.get_modifier4_id());
    return (1);
}

FT_TEST(test_game_item_edge_modifier_values)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    item.set_modifier1_value(-1);
    item.set_modifier2_value(2);
    item.set_modifier3_value(3);
    item.set_modifier4_value(4);
    FT_ASSERT_EQ(-1, item.get_modifier1_value());
    FT_ASSERT_EQ(4, item.get_modifier4_value());
    return (1);
}

FT_TEST(test_game_item_edge_thread_safety_round_trip)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, item.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_item_edge_destroy_reinitialize)
{
    game_item item;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    item.set_item_id(42);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    FT_ASSERT_EQ(0, item.get_item_id());
    return (1);
}

FT_TEST(test_game_inventory_edge_zero_capacity)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(0, 0));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), inventory.get_capacity());
    FT_ASSERT_EQ(FT_FALSE, inventory.is_full());
    return (1);
}

FT_TEST(test_game_inventory_edge_negative_weight_limit)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(2, -1));
    inventory.set_weight_limit(-4);
    FT_ASSERT_EQ(-4, inventory.get_weight_limit());
    return (1);
}

FT_TEST(test_game_inventory_edge_set_used_slots)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(5, 0));
    inventory.set_used_slots(3);
    FT_ASSERT_EQ(static_cast<ft_size_t>(3), inventory.get_used());
    FT_ASSERT_EQ(FT_FALSE, inventory.is_full());
    return (1);
}

FT_TEST(test_game_inventory_edge_resize_down)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(5, 0));
    inventory.resize(2);
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), inventory.get_capacity());
    return (1);
}

FT_TEST(test_game_inventory_edge_missing_item_count)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(2, 0));
    FT_ASSERT_EQ(0, inventory.count_item(999));
    FT_ASSERT_EQ(FT_FALSE, inventory.has_item(999));
    return (1);
}

FT_TEST(test_game_inventory_edge_missing_rarity_count)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(2, 0));
    FT_ASSERT_EQ(0, inventory.count_rarity(999));
    FT_ASSERT_EQ(FT_FALSE, inventory.has_rarity(999));
    return (1);
}

FT_TEST(test_game_inventory_edge_remove_missing_slot)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(2, 0));
    inventory.remove_item(100);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_game_inventory_edge_copy_empty)
{
    game_inventory source;
    game_inventory destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, 12));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(static_cast<ft_size_t>(7), destination.get_capacity());
    FT_ASSERT_EQ(12, destination.get_weight_limit());
    return (1);
}

FT_TEST(test_game_inventory_edge_move_empty)
{
    game_inventory source;
    game_inventory destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(8, 13));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_inventory &&>(source)));
    FT_ASSERT_EQ(static_cast<ft_size_t>(8), destination.get_capacity());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_inventory_edge_thread_safety_round_trip)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(2, 0));
    FT_ASSERT_EQ(FT_FALSE, inventory.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, inventory.is_thread_safe());
    return (1);
}

FT_TEST(test_game_inventory_edge_empty_items_view)
{
    game_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(3, 0));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), inventory.get_items().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}
