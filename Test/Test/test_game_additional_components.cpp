#include "../test_internal.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_behavior_action.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_inventory_weight_limit_blocks_heavy_stack, "Game: inventory enforces weight limits when adding heavy stacks")
{
    ft_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(5, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    ft_sharedptr<ft_item> heavy(new ft_item());
    int result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(5, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    heavy->set_item_id(1);
    heavy->set_max_stack(10);
    heavy->set_stack_size(6);
    result = inventory.add_item(heavy);
    FT_ASSERT_EQ(FT_ERR_FULL, result);
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.get_error());
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_price_definition_copy_independent_after_mutation, "Game: price definition copy remains stable after source changes")
{
    ft_price_definition original;
    ft_price_definition duplicate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(11, 2, 120, 80, 200));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());

    original.set_base_value(150);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_minimum_value(60);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_maximum_value(250);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(11, duplicate.get_item_id());
    FT_ASSERT_EQ(2, duplicate.get_rarity());
    FT_ASSERT_EQ(120, duplicate.get_base_value());
    FT_ASSERT_EQ(80, duplicate.get_minimum_value());
    FT_ASSERT_EQ(200, duplicate.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_move_assignment_clears_source, "Game: moving currency rates resets the origin values")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, 2.5, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(7, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(2.5, destination.get_rate_to_base());
    FT_ASSERT_EQ(4, destination.get_display_precision());
    FT_ASSERT_EQ(0, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_rate_to_base());
    FT_ASSERT_EQ(0, source.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_behavior_action_setters_update_values, "Game: behavior action setters store the provided parameters")
{
    ft_behavior_action action;

    action.set_action_id(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    action.set_weight(1.75);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    action.set_cooldown_seconds(3.5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    FT_ASSERT_EQ(9, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.75, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(3.5, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_region_definition_assignment_isolated_from_source, "Game: region definition assignment keeps independent copies")
{
    ft_region_definition original;
    ft_region_definition assigned;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(3, ft_string("Highlands"),
        ft_string("Jagged peaks"), 15));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.get_error());
    original.set_name(ft_string("Lowlands"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_description(ft_string("Rolling plains"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_recommended_level(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(3, assigned.get_region_id());
    FT_ASSERT_STR_EQ("Highlands", assigned.get_name().c_str());
    FT_ASSERT_STR_EQ("Jagged peaks", assigned.get_description().c_str());
    FT_ASSERT_EQ(15, assigned.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.get_error());
    return (1);
}

FT_TEST(test_price_definition_move_assignment_resets_source, "Game: moving price definitions clears the origin values")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4, 3, 180, 90, 240));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(9, 1, 50, 25, 100));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(4, destination.get_item_id());
    FT_ASSERT_EQ(3, destination.get_rarity());
    FT_ASSERT_EQ(180, destination.get_base_value());
    FT_ASSERT_EQ(90, destination.get_minimum_value());
    FT_ASSERT_EQ(240, destination.get_maximum_value());
    FT_ASSERT_EQ(0, source.get_item_id());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_EQ(0, source.get_base_value());
    FT_ASSERT_EQ(0, source.get_minimum_value());
    FT_ASSERT_EQ(0, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_currency_rate_copy_remains_unchanged_after_source_update, "Game: currency rate copies stay stable when source mutate")
{
    ft_currency_rate original;
    ft_currency_rate duplicate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(6, 1.75, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());

    original.set_currency_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_rate_to_base(3.5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    original.set_display_precision(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(6, duplicate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.75, duplicate.get_rate_to_base());
    FT_ASSERT_EQ(5, duplicate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    return (1);
}

FT_TEST(test_region_definition_move_assignment_clears_source, "Game: region definition move assignment resets origin fields")
{
    ft_region_definition source;
    ft_region_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(8, ft_string("Harbor"),
        ft_string("Dockyards"), 7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(8, destination.get_region_id());
    FT_ASSERT_EQ(ft_string("Harbor"), destination.get_name());
    FT_ASSERT_EQ(ft_string("Dockyards"), destination.get_description());
    FT_ASSERT_EQ(7, destination.get_recommended_level());
    FT_ASSERT_EQ(0, source.get_region_id());
    FT_ASSERT_EQ(ft_string(""), source.get_name());
    FT_ASSERT_EQ(ft_string(""), source.get_description());
    FT_ASSERT_EQ(0, source.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}
