#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_behavior_action.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_definition_defaults_zero, "ft_price_definition defaults to zeros")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_setters_apply_values, "ft_price_definition setters override stored fields")
{
    ft_price_definition definition;

    definition.set_item_id(8);
    definition.set_rarity(2);
    definition.set_base_value(180);
    definition.set_minimum_value(140);
    definition.set_maximum_value(220);
    FT_ASSERT_EQ(8, definition.get_item_id());
    FT_ASSERT_EQ(2, definition.get_rarity());
    FT_ASSERT_EQ(180, definition.get_base_value());
    FT_ASSERT_EQ(140, definition.get_minimum_value());
    FT_ASSERT_EQ(220, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_currency_rate_defaults_and_setters, "ft_currency_rate exposes base fields and setters")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());

    rate.set_currency_id(12);
    rate.set_rate_to_base(3.75);
    rate.set_display_precision(5);
    FT_ASSERT_EQ(12, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    return (1);
}

FT_TEST(test_behavior_action_setters, "ft_behavior_action setters update the stored configuration")
{
    ft_behavior_action action;

    action.set_action_id(7);
    action.set_weight(4.5);
    action.set_cooldown_seconds(2.25);
    FT_ASSERT_EQ(7, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(4.5, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(2.25, action.get_cooldown_seconds());

    ft_rarity_band rarity;
    rarity.set_rarity(3);
    rarity.set_value_multiplier(2.5);
    FT_ASSERT_EQ(3, rarity.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.5, rarity.get_value_multiplier());
    return (1);
}
