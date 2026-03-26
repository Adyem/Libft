#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static game_price_definition g_uninitialised_table_register_definition;
static game_price_definition g_uninitialised_table_fetch_definition;

static int expect_sigabrt_on_uninitialised_table(void (*operation)(game_economy_table &))
{
    int result;

    result = test_expect_sigabrt_signal_uninitialised<game_economy_table>(
        operation);
    (void)g_uninitialised_table_register_definition.destroy();
    (void)g_uninitialised_table_fetch_definition.destroy();
    return (result);
}

static void uninitialised_table_register_price_operation(game_economy_table &table)
{
    int result;

    (void)g_uninitialised_table_register_definition.destroy();
    result = g_uninitialised_table_register_definition.initialize(10, 3, 500, 300, 800);
    if (result != FT_ERR_SUCCESS)
        return ;
    (void)table.register_price_definition(g_uninitialised_table_register_definition);
    return ;
}

static void uninitialised_table_fetch_price_operation(game_economy_table &table)
{
    int result;

    (void)g_uninitialised_table_fetch_definition.destroy();
    result = g_uninitialised_table_fetch_definition.initialize();
    if (result != FT_ERR_SUCCESS)
        return ;
    (void)table.fetch_price_definition(10, g_uninitialised_table_fetch_definition);
    return ;
}

static int assert_price_definition(const game_price_definition &definition,
        int item_id, int rarity, int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

static int assert_vendor_profile_values(const game_vendor_profile &profile,
        int vendor_id, double buy_markup, double sell_multiplier, double tax_rate)
{
    FT_ASSERT_EQ(vendor_id, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(buy_markup, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(sell_multiplier, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(tax_rate, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_economy_register_and_fetch_price)
{
    game_economy_table table;
    game_price_definition definition;
    game_price_definition fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(10, 3, 500, 300, 800));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_price_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_price_definition(10, fetched));
    assert_price_definition(fetched, 10, 3, 500, 300, 800);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_economy_missing_entries)
{
    game_economy_table table;
    game_price_definition definition;
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_price_definition(99, definition));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_currency_rate(7, rate));
    return (1);
}

FT_TEST(test_economy_rarity_band_registration)
{
    game_economy_table table;
    game_rarity_band band;
    game_rarity_band fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(4, 1.75));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_rarity_band(band));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_rarity_band(4, fetched));
    FT_ASSERT_EQ(4, fetched.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.75, fetched.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_economy_vendor_and_currency_profiles)
{
    game_economy_table table;
    game_vendor_profile vendor;
    game_currency_rate currency;
    game_vendor_profile fetched_vendor;
    game_currency_rate fetched_currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor.initialize(2, 1.25, 0.60, 0.05));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize(1, 0.5, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_currency_rate(currency));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_vendor_profile(2, fetched_vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_currency_rate(1, fetched_currency));
    assert_vendor_profile_values(fetched_vendor, 2, 1.25, 0.60, 0.05);
    FT_ASSERT_EQ(1, fetched_currency.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, fetched_currency.get_rate_to_base());
    FT_ASSERT_EQ(3, fetched_currency.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_vendor.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_currency.get_error());
    return (1);
}

FT_TEST(test_economy_register_price_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(
        uninitialised_table_register_price_operation));
    return (1);
}

FT_TEST(test_economy_fetch_price_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(
        uninitialised_table_fetch_price_operation));
    return (1);
}
