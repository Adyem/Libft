#include "../test_internal.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_currency_rate_default_constructor_values, "Currency rate default constructor initializes baseline values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_initialize_with_fields_values, "Currency rate initialize stores provided values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(42, 3.75, 5));
    FT_ASSERT_EQ(42, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_initialize_copy_clones_values, "Currency rate initialize(copy) matches source state")
{
    ft_currency_rate original;
    ft_currency_rate copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(8, 0.25, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(original));

    FT_ASSERT_EQ(8, copy.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.25, copy.get_rate_to_base());
    FT_ASSERT_EQ(4, copy.get_display_precision());
    FT_ASSERT_EQ(8, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.25, original.get_rate_to_base());
    FT_ASSERT_EQ(4, original.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_initialize_overwrites_destination, "Currency rate initialize(copy) overwrites existing values")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(13, 1.2, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(5, 0.5, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));

    FT_ASSERT_EQ(13, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.2, destination.get_rate_to_base());
    FT_ASSERT_EQ(6, destination.get_display_precision());
    FT_ASSERT_EQ(5, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, source.get_rate_to_base());
    FT_ASSERT_EQ(1, source.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_setters_update_values, "Currency rate setters update stored fields")
{
    ft_currency_rate rate;

    rate.set_currency_id(77);
    rate.set_rate_to_base(1.75);
    rate.set_display_precision(8);

    FT_ASSERT_EQ(77, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(8, rate.get_display_precision());
    return (1);
}


FT_TEST(test_price_definition_setters_apply_updates, "Price definition setters update stored values")
{
    ft_price_definition definition;

    definition.set_item_id(11);
    definition.set_rarity(5);
    definition.set_base_value(220);
    definition.set_minimum_value(120);
    definition.set_maximum_value(400);

    FT_ASSERT_EQ(11, definition.get_item_id());
    FT_ASSERT_EQ(5, definition.get_rarity());
    FT_ASSERT_EQ(220, definition.get_base_value());
    FT_ASSERT_EQ(120, definition.get_minimum_value());
    FT_ASSERT_EQ(400, definition.get_maximum_value());
    return (1);
}
