#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/move.hpp"

static int assert_price_definition_values(const ft_price_definition &definition, int item_id,
        int rarity, int base_value, int minimum_value, int maximum_value)
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

FT_TEST(test_economy_table_setters_copy_maps, "setters copy maps without aliasing")
{
    ft_economy_table table;
    ft_map<int, ft_price_definition> price_map;
    ft_map<int, ft_rarity_band> rarity_map;
    ft_map<int, ft_vendor_profile> vendor_map;
    ft_map<int, ft_currency_rate> currency_map;
    ft_price_definition price_entry(3, 2, 500, 300, 900);
    ft_rarity_band rarity_entry(6, 1.40);
    ft_vendor_profile vendor_entry(12, 1.10, 0.65, 0.03);
    ft_currency_rate currency_entry(8, 0.75, 2);
    ft_price_definition fetched_price;
    ft_rarity_band fetched_rarity;
    ft_vendor_profile fetched_vendor;
    ft_currency_rate fetched_currency;

    price_map.insert(3, price_entry);
    rarity_map.insert(6, rarity_entry);
    vendor_map.insert(12, vendor_entry);
    currency_map.insert(8, currency_entry);
    table.set_price_definitions(price_map);
    table.set_rarity_bands(rarity_map);
    table.set_vendor_profiles(ft_move(vendor_map));
    table.set_currency_rates(currency_map);
    price_map.clear();
    rarity_map.clear();
    vendor_map.clear();
    currency_map.clear();
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_price_definition(3, fetched_price));
    assert_price_definition_values(fetched_price, 3, 2, 500, 300, 900);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_rarity_band(6, fetched_rarity));
    FT_ASSERT_EQ(6, fetched_rarity.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.40, fetched_rarity.get_value_multiplier());
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_vendor_profile(12, fetched_vendor));
    assert_vendor_profile_values(fetched_vendor, 12, 1.10, 0.65, 0.03);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_currency_rate(8, fetched_currency));
    FT_ASSERT_EQ(8, fetched_currency.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.75, fetched_currency.get_rate_to_base());
    FT_ASSERT_EQ(2, fetched_currency.get_display_precision());
    return (1);
}

FT_TEST(test_economy_table_setters_replace_records, "setters replace previous entries")
{
    ft_economy_table table;
    ft_map<int, ft_price_definition> initial_prices;
    ft_map<int, ft_price_definition> replacement_prices;
    ft_map<int, ft_vendor_profile> initial_vendors;
    ft_map<int, ft_vendor_profile> replacement_vendors;
    ft_price_definition original_price(1, 1, 200, 150, 300);
    ft_price_definition new_price(2, 3, 800, 500, 1200);
    ft_vendor_profile original_vendor(4, 1.20, 0.55, 0.07);
    ft_vendor_profile new_vendor(9, 1.35, 0.60, 0.06);
    ft_price_definition fetched_price;
    ft_vendor_profile fetched_vendor;

    initial_prices.insert(1, original_price);
    table.set_price_definitions(initial_prices);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_price_definition(1, fetched_price));
    assert_price_definition_values(fetched_price, 1, 1, 200, 150, 300);
    replacement_prices.insert(2, new_price);
    table.set_price_definitions(replacement_prices);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_price_definition(2, fetched_price));
    assert_price_definition_values(fetched_price, 2, 3, 800, 500, 1200);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_price_definition(1, fetched_price));
    initial_vendors.insert(4, original_vendor);
    table.set_vendor_profiles(ft_move(initial_vendors));
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_vendor_profile(4, fetched_vendor));
    assert_vendor_profile_values(fetched_vendor, 4, 1.20, 0.55, 0.07);
    replacement_vendors.insert(9, new_vendor);
    table.set_vendor_profiles(ft_move(replacement_vendors));
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_vendor_profile(9, fetched_vendor));
    assert_vendor_profile_values(fetched_vendor, 9, 1.35, 0.60, 0.06);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_vendor_profile(4, fetched_vendor));
    return (1);
}
