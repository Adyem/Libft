#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_definition_default_state, "Price definition constructor zeroes all fields")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_initialize_with_fields, "Price definition initialize stores provided inputs")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(19, 3, 240, 120, 360));
    FT_ASSERT_EQ(19, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(240, definition.get_base_value());
    FT_ASSERT_EQ(120, definition.get_minimum_value());
    FT_ASSERT_EQ(360, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_setters_update_all_fields, "Price definition setters alter every field")
{
    ft_price_definition definition;

    definition.set_item_id(7);
    definition.set_rarity(5);
    definition.set_base_value(810);
    definition.set_minimum_value(200);
    definition.set_maximum_value(950);

    FT_ASSERT_EQ(7, definition.get_item_id());
    FT_ASSERT_EQ(5, definition.get_rarity());
    FT_ASSERT_EQ(810, definition.get_base_value());
    FT_ASSERT_EQ(200, definition.get_minimum_value());
    FT_ASSERT_EQ(950, definition.get_maximum_value());
    return (1);
}

FT_TEST(test_price_definition_initialize_copy_matches_source, "Price definition initialize(copy) matches the source fields")
{
    ft_price_definition original;
    ft_price_definition dest;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(22, 4, 150, 90, 400));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dest.initialize(original));

    FT_ASSERT_EQ(22, dest.get_item_id());
    FT_ASSERT_EQ(4, dest.get_rarity());
    FT_ASSERT_EQ(150, dest.get_base_value());
    FT_ASSERT_EQ(90, dest.get_minimum_value());
    FT_ASSERT_EQ(400, dest.get_maximum_value());
    FT_ASSERT_EQ(22, original.get_item_id());
    FT_ASSERT_EQ(4, original.get_rarity());
    FT_ASSERT_EQ(150, original.get_base_value());
    FT_ASSERT_EQ(90, original.get_minimum_value());
    FT_ASSERT_EQ(400, original.get_maximum_value());
    return (1);
}

FT_TEST(test_currency_rate_default_state, "Currency rate default constructor preloads base values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_initialize_with_fields_values, "Currency rate initialize captures provided values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(17, 2.75, 5));
    FT_ASSERT_EQ(17, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(2.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_setters_update_fields, "Currency rate setters mutate identifiers and precision")
{
    ft_currency_rate rate;

    rate.set_currency_id(9);
    rate.set_rate_to_base(0.65);
    rate.set_display_precision(6);

    FT_ASSERT_EQ(9, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.65, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    return (1);
}

FT_TEST(test_currency_rate_initialize_copy_overwrites_state, "Currency rate initialize(copy) overwrites previous values")
{
    ft_currency_rate source;
    ft_currency_rate target;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 1.1, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target.initialize(8, 0.5, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target.initialize(source));

    FT_ASSERT_EQ(3, target.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.1, target.get_rate_to_base());
    FT_ASSERT_EQ(4, target.get_display_precision());
    FT_ASSERT_EQ(3, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.1, source.get_rate_to_base());
    FT_ASSERT_EQ(4, source.get_display_precision());
    return (1);
}
