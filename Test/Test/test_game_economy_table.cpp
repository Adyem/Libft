#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

static int assert_price_definition(const ft_price_definition &definition, int item_id, int rarity,
        int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    return (1);
}

static int assert_vendor_profile_values(const ft_vendor_profile &profile, int vendor_id,
        double buy_markup, double sell_multiplier, double tax_rate)
{
    FT_ASSERT_EQ(vendor_id, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(buy_markup, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(sell_multiplier, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(tax_rate, profile.get_tax_rate());
    return (1);
}

FT_TEST(test_economy_register_and_fetch_price, "register and fetch price definition")
{
    ft_economy_table table;
    ft_price_definition definition(10, 3, 500, 300, 800);
    ft_price_definition fetched;

    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.register_price_definition(definition));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.fetch_price_definition(10, fetched));
    assert_price_definition(fetched, 10, 3, 500, 300, 800);
    return (1);
}

FT_TEST(test_economy_missing_entries, "missing entries return not found")
{
    ft_economy_table table;
    ft_price_definition definition;
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_price_definition(99, definition));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_currency_rate(7, rate));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_economy_rarity_band_registration, "register and fetch rarity band")
{
    ft_economy_table table;
    ft_rarity_band band(4, 1.75);
    ft_rarity_band fetched;

    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.register_rarity_band(band));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.fetch_rarity_band(4, fetched));
    FT_ASSERT_EQ(4, fetched.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.75, fetched.get_value_multiplier());
    return (1);
}

FT_TEST(test_economy_vendor_and_currency_profiles, "register vendor and currency profiles")
{
    ft_economy_table table;
    ft_vendor_profile vendor(2, 1.25, 0.60, 0.05);
    ft_currency_rate currency(1, 0.5, 3);
    ft_vendor_profile fetched_vendor;
    ft_currency_rate fetched_currency;

    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.register_currency_rate(currency));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.fetch_vendor_profile(2, fetched_vendor));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, table.fetch_currency_rate(1, fetched_currency));
    assert_vendor_profile_values(fetched_vendor, 2, 1.25, 0.60, 0.05);
    FT_ASSERT_EQ(1, fetched_currency.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, fetched_currency.get_rate_to_base());
    FT_ASSERT_EQ(3, fetched_currency.get_display_precision());
    return (1);
}
