#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int register_all_records(ft_economy_table &table)
{
    ft_price_definition definition(4, 2, 900, 400, 1200);
    ft_rarity_band band(7, 1.15);
    ft_vendor_profile vendor(11, 1.30, 0.55, 0.08);
    ft_currency_rate rate(5, 2.50, 4);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_price_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_rarity_band(band));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_currency_rate(rate));
    return (1);
}

FT_TEST(test_economy_table_initialize_copy, "economy table initialize(copy) duplicates entries")
{
    ft_economy_table source;
    ft_economy_table duplicate;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(source));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_currency_rate(5, currency));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_currency_rate(5, currency));
    return (1);
}

FT_TEST(test_economy_table_initialize_overwrites_existing_entries, "economy table initialize(copy) replaces current entries")
{
    ft_economy_table source;
    ft_economy_table destination;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    register_all_records(destination);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_currency_rate(5, currency));
    return (1);
}

FT_TEST(test_economy_table_initialize_move_transfers_entries, "economy table initialize(move) transfers entries and clears source")
{
    ft_economy_table source;
    ft_economy_table moved;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(ft_move(source)));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_currency_rate(5, currency));
    FT_ASSERT(source.get_price_definitions().empty());
    FT_ASSERT(source.get_rarity_bands().empty());
    FT_ASSERT(source.get_vendor_profiles().empty());
    FT_ASSERT(source.get_currency_rates().empty());
    return (1);
}
