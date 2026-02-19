#include "../test_internal.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_currency_rate_default_initialization_values, "Currency rate default constructor initializes base state")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_parameterized_constructor_values, "Currency rate parameterized constructor stores fields")
{
    ft_currency_rate rate(4, 0.5, 6);

    FT_ASSERT_EQ(4, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_initialize_copy_resets_destination, "Currency rate initialize(copy) overwrites previous state but keeps source")
{
    ft_currency_rate original(8, 1.35, 5);
    ft_currency_rate destination(2, 0.4, 3);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(original));

    FT_ASSERT_EQ(8, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.35, destination.get_rate_to_base());
    FT_ASSERT_EQ(5, destination.get_display_precision());
    FT_ASSERT_EQ(8, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.35, original.get_rate_to_base());
    FT_ASSERT_EQ(5, original.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_setters_apply_values, "Currency rate setters update stored values")
{
    ft_currency_rate rate;

    rate.set_currency_id(13);
    rate.set_rate_to_base(4.25);
    rate.set_display_precision(0);

    FT_ASSERT_EQ(13, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.25, rate.get_rate_to_base());
    FT_ASSERT_EQ(0, rate.get_display_precision());
    return (1);
}

FT_TEST(test_price_definition_default_values, "Price definition default constructor zeroes all fields")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_parameterized_constructor_values, "Price definition parameterized constructor stores inputs")
{
    ft_price_definition definition(10, 3, 700, 450, 900);

    FT_ASSERT_EQ(10, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(700, definition.get_base_value());
    FT_ASSERT_EQ(450, definition.get_minimum_value());
    FT_ASSERT_EQ(900, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_initialize_copy_matches_source, "Price definition initialize(copy) duplicates source state")
{
    ft_price_definition original(12, 5, 250, 100, 400);
    ft_price_definition clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(original));

    FT_ASSERT_EQ(12, clone.get_item_id());
    FT_ASSERT_EQ(5, clone.get_rarity());
    FT_ASSERT_EQ(250, clone.get_base_value());
    FT_ASSERT_EQ(100, clone.get_minimum_value());
    FT_ASSERT_EQ(400, clone.get_maximum_value());
    FT_ASSERT_EQ(12, original.get_item_id());
    FT_ASSERT_EQ(5, original.get_rarity());
    FT_ASSERT_EQ(250, original.get_base_value());
    FT_ASSERT_EQ(100, original.get_minimum_value());
    FT_ASSERT_EQ(400, original.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_setters_apply_updates, "Price definition setters adjust all value fields")
{
    ft_price_definition definition;

    definition.set_item_id(21);
    definition.set_rarity(7);
    definition.set_base_value(1500);
    definition.set_minimum_value(1200);
    definition.set_maximum_value(1800);

    FT_ASSERT_EQ(21, definition.get_item_id());
    FT_ASSERT_EQ(7, definition.get_rarity());
    FT_ASSERT_EQ(1500, definition.get_base_value());
    FT_ASSERT_EQ(1200, definition.get_minimum_value());
    FT_ASSERT_EQ(1800, definition.get_maximum_value());
    return (1);
}
