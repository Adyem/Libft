#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_behavior_action.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_definition_default_values, "Game: price definition defaults to zeroed values")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_move_assignment_resets_origin, "Game: moving price definitions clears the source state")
{
    ft_price_definition source(9, 3, 450, 300, 600);
    ft_price_definition destination;

    destination = ft_move(source);
    FT_ASSERT_EQ(9, destination.get_item_id());
    FT_ASSERT_EQ(3, destination.get_rarity());
    FT_ASSERT_EQ(450, destination.get_base_value());
    FT_ASSERT_EQ(300, destination.get_minimum_value());
    FT_ASSERT_EQ(600, destination.get_maximum_value());
    FT_ASSERT_EQ(0, source.get_item_id());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_EQ(0, source.get_base_value());
    FT_ASSERT_EQ(0, source.get_minimum_value());
    FT_ASSERT_EQ(0, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_currency_rate_default_configuration, "Game: currency rate provides sensible defaults")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_setters_replace_values, "Game: currency rate setters override previous configuration")
{
    ft_currency_rate rate;

    rate.set_currency_id(14);
    rate.set_rate_to_base(3.25);
    rate.set_display_precision(5);
    FT_ASSERT_EQ(14, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.25, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_behavior_action_copy_isolated_from_source_mutation, "Game: behavior action copies remain stable after source updates")
{
    ft_behavior_action original(4, 1.0, 2.0);
    ft_behavior_action duplicate(original);

    original.set_action_id(8);
    original.set_weight(5.0);
    original.set_cooldown_seconds(7.5);
    FT_ASSERT_EQ(4, duplicate.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.0, duplicate.get_weight());
    FT_ASSERT_DOUBLE_EQ(2.0, duplicate.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    return (1);
}

FT_TEST(test_price_definition_copy_constructor_duplicates_values, "Game: price definition copy constructor mirrors source values")
{
    ft_price_definition original(15, 2, 120, 100, 180);
    ft_price_definition replica(original);

    FT_ASSERT_EQ(15, replica.get_item_id());
    FT_ASSERT_EQ(2, replica.get_rarity());
    FT_ASSERT_EQ(120, replica.get_base_value());
    FT_ASSERT_EQ(100, replica.get_minimum_value());
    FT_ASSERT_EQ(180, replica.get_maximum_value());
    FT_ASSERT_EQ(15, original.get_item_id());
    FT_ASSERT_EQ(2, original.get_rarity());
    FT_ASSERT_EQ(120, original.get_base_value());
    FT_ASSERT_EQ(100, original.get_minimum_value());
    FT_ASSERT_EQ(180, original.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, replica.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_price_definition_setters_update_all_fields, "Game: price definition setters override configured values")
{
    ft_price_definition definition(5, 1, 75, 60, 90);

    definition.set_item_id(6);
    definition.set_rarity(3);
    definition.set_base_value(110);
    definition.set_minimum_value(95);
    definition.set_maximum_value(150);
    FT_ASSERT_EQ(6, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(110, definition.get_base_value());
    FT_ASSERT_EQ(95, definition.get_minimum_value());
    FT_ASSERT_EQ(150, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_currency_rate_move_constructor_clears_source, "Game: moving currency rates resets the origin to defaults")
{
    ft_currency_rate original(21, 2.5, 4);
    ft_currency_rate moved(ft_move(original));

    FT_ASSERT_EQ(21, moved.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(2.5, moved.get_rate_to_base());
    FT_ASSERT_EQ(4, moved.get_display_precision());
    FT_ASSERT_EQ(0, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, original.get_rate_to_base());
    FT_ASSERT_EQ(0, original.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_rarity_band_default_configuration, "Game: rarity band defaults to neutral multiplier")
{
    ft_rarity_band rarity_band;

    FT_ASSERT_EQ(0, rarity_band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.0, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_behavior_action_move_assignment_zeros_origin, "Game: move assigning behavior actions clears the source values")
{
    ft_behavior_action source(6, 3.5, 9.0);
    ft_behavior_action destination;

    destination = ft_move(source);
    FT_ASSERT_EQ(6, destination.get_action_id());
    FT_ASSERT_DOUBLE_EQ(3.5, destination.get_weight());
    FT_ASSERT_DOUBLE_EQ(9.0, destination.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_price_definition_copy_assignment_transfers_fields, "Game: copy assigning price definitions duplicates all values")
{
    ft_price_definition source(11, 5, 900, 750, 1200);
    ft_price_definition destination;

    destination = source;
    FT_ASSERT_EQ(11, destination.get_item_id());
    FT_ASSERT_EQ(5, destination.get_rarity());
    FT_ASSERT_EQ(900, destination.get_base_value());
    FT_ASSERT_EQ(750, destination.get_minimum_value());
    FT_ASSERT_EQ(1200, destination.get_maximum_value());
    FT_ASSERT_EQ(11, source.get_item_id());
    FT_ASSERT_EQ(5, source.get_rarity());
    FT_ASSERT_EQ(900, source.get_base_value());
    FT_ASSERT_EQ(750, source.get_minimum_value());
    FT_ASSERT_EQ(1200, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_price_definition_move_constructor_clears_source_state, "Game: move constructing price definitions resets the origin")
{
    ft_price_definition original(20, 7, 1500, 1300, 1800);
    ft_price_definition moved(ft_move(original));

    FT_ASSERT_EQ(20, moved.get_item_id());
    FT_ASSERT_EQ(7, moved.get_rarity());
    FT_ASSERT_EQ(1500, moved.get_base_value());
    FT_ASSERT_EQ(1300, moved.get_minimum_value());
    FT_ASSERT_EQ(1800, moved.get_maximum_value());
    FT_ASSERT_EQ(0, original.get_item_id());
    FT_ASSERT_EQ(0, original.get_rarity());
    FT_ASSERT_EQ(0, original.get_base_value());
    FT_ASSERT_EQ(0, original.get_minimum_value());
    FT_ASSERT_EQ(0, original.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_currency_rate_copy_assignment_preserves_source, "Game: copy assignment copies currency rates without altering the source")
{
    ft_currency_rate source(17, 4.5, 6);
    ft_currency_rate destination;

    destination = source;
    FT_ASSERT_EQ(17, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.5, destination.get_rate_to_base());
    FT_ASSERT_EQ(6, destination.get_display_precision());
    FT_ASSERT_EQ(17, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.5, source.get_rate_to_base());
    FT_ASSERT_EQ(6, source.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_rarity_band_move_assignment_resets_origin, "Game: move assigning rarity bands clears the original values")
{
    ft_rarity_band source(9, 3.0);
    ft_rarity_band destination;

    destination = ft_move(source);
    FT_ASSERT_EQ(9, destination.get_rarity());
    FT_ASSERT_DOUBLE_EQ(3.0, destination.get_value_multiplier());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_behavior_action_setters_replace_existing_values, "Game: behavior action setters overwrite previous configuration")
{
    ft_behavior_action action;

    action.set_action_id(12);
    action.set_weight(8.5);
    action.set_cooldown_seconds(4.0);
    FT_ASSERT_EQ(12, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(8.5, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(4.0, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_currency_rate_move_assignment_resets_origin_state, "Game: move assigning currency rates resets the source values")
{
    ft_currency_rate source(13, 6.75, 7);
    ft_currency_rate destination;

    destination = ft_move(source);
    FT_ASSERT_EQ(13, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(6.75, destination.get_rate_to_base());
    FT_ASSERT_EQ(7, destination.get_display_precision());
    FT_ASSERT_EQ(0, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_rate_to_base());
    FT_ASSERT_EQ(0, source.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_rarity_band_copy_constructor_preserves_source, "Game: rarity band copy constructor leaves the source intact")
{
    ft_rarity_band original(4, 2.25);
    ft_rarity_band copy(original);

    FT_ASSERT_EQ(4, copy.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.25, copy.get_value_multiplier());
    FT_ASSERT_EQ(4, original.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.25, original.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_override_values, "Game: rarity band setters replace default configuration")
{
    ft_rarity_band band;

    band.set_rarity(6);
    band.set_value_multiplier(3.5);
    FT_ASSERT_EQ(6, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(3.5, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_behavior_action_move_constructor_clears_origin, "Game: move constructing behavior actions resets the source state")
{
    ft_behavior_action original(10, 0.5, 12.0);
    ft_behavior_action moved(ft_move(original));

    FT_ASSERT_EQ(10, moved.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.5, moved.get_weight());
    FT_ASSERT_DOUBLE_EQ(12.0, moved.get_cooldown_seconds());
    FT_ASSERT_EQ(0, original.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, original.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, original.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_price_definition_self_assignment_retains_values, "Game: self assigning price definition keeps values unchanged")
{
    ft_price_definition definition(7, 2, 200, 150, 260);

    definition = definition;
    FT_ASSERT_EQ(7, definition.get_item_id());
    FT_ASSERT_EQ(2, definition.get_rarity());
    FT_ASSERT_EQ(200, definition.get_base_value());
    FT_ASSERT_EQ(150, definition.get_minimum_value());
    FT_ASSERT_EQ(260, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_currency_rate_parameter_constructor_sets_all_fields, "Game: currency rate constructor applies provided values")
{
    ft_currency_rate rate(32, 1.75, 3);

    FT_ASSERT_EQ(32, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(3, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_self_assignment_keeps_configuration, "Game: currency rate self assignment preserves values")
{
    ft_currency_rate rate(44, 9.25, 1);

    rate = rate;
    FT_ASSERT_EQ(44, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(9.25, rate.get_rate_to_base());
    FT_ASSERT_EQ(1, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_rarity_band_copy_assignment_updates_destination, "Game: copy assigning rarity bands clones values")
{
    ft_rarity_band source(11, 4.5);
    ft_rarity_band destination(2, 0.75);

    destination = source;
    FT_ASSERT_EQ(11, destination.get_rarity());
    FT_ASSERT_DOUBLE_EQ(4.5, destination.get_value_multiplier());
    FT_ASSERT_EQ(11, source.get_rarity());
    FT_ASSERT_DOUBLE_EQ(4.5, source.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_price_definition_parameter_constructor_initializes_values, "Game: price definition constructor sets all provided data")
{
    ft_price_definition definition(22, 9, 1750, 1600, 2000);

    FT_ASSERT_EQ(22, definition.get_item_id());
    FT_ASSERT_EQ(9, definition.get_rarity());
    FT_ASSERT_EQ(1750, definition.get_base_value());
    FT_ASSERT_EQ(1600, definition.get_minimum_value());
    FT_ASSERT_EQ(2000, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_behavior_action_self_assignment_leaves_values, "Game: behavior action self assignment keeps configuration intact")
{
    ft_behavior_action action(18, 6.0, 14.0);

    action = action;
    FT_ASSERT_EQ(18, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(6.0, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(14.0, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}
