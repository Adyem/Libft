#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_price_definition_setters, "update price definition fields")
{
    ft_price_definition definition;

    definition.set_item_id(11);
    definition.set_rarity(2);
    definition.set_base_value(750);
    definition.set_minimum_value(500);
    definition.set_maximum_value(1200);

    FT_ASSERT_EQ(11, definition.get_item_id());
    FT_ASSERT_EQ(2, definition.get_rarity());
    FT_ASSERT_EQ(750, definition.get_base_value());
    FT_ASSERT_EQ(500, definition.get_minimum_value());
    FT_ASSERT_EQ(1200, definition.get_maximum_value());
    FT_ASSERT_EQ(ER_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters, "update rarity band multiplier")
{
    ft_rarity_band band;

    band.set_rarity(6);
    band.set_value_multiplier(2.25);

    FT_ASSERT_EQ(6, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.25, band.get_value_multiplier());
    FT_ASSERT_EQ(ER_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_setters, "update vendor profile attributes")
{
    ft_vendor_profile vendor;

    vendor.set_vendor_id(4);
    vendor.set_buy_markup(1.4);
    vendor.set_sell_multiplier(0.55);
    vendor.set_tax_rate(0.08);

    FT_ASSERT_EQ(4, vendor.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.4, vendor.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.55, vendor.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.08, vendor.get_tax_rate());
    FT_ASSERT_EQ(ER_SUCCESS, vendor.get_error());
    return (1);
}

FT_TEST(test_currency_rate_setters, "update currency rate fields")
{
    ft_currency_rate rate;

    rate.set_currency_id(3);
    rate.set_rate_to_base(1.2);
    rate.set_display_precision(4);

    FT_ASSERT_EQ(3, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.2, rate.get_rate_to_base());
    FT_ASSERT_EQ(4, rate.get_display_precision());
    FT_ASSERT_EQ(ER_SUCCESS, rate.get_error());
    return (1);
}
