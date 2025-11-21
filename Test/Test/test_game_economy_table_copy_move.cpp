#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

static int register_all_records(ft_economy_table &table)
{
    ft_price_definition definition(4, 2, 900, 400, 1200);
    ft_rarity_band band(7, 1.15);
    ft_vendor_profile vendor(11, 1.30, 0.55, 0.08);
    ft_currency_rate rate(5, 2.50, 4);

    FT_ASSERT_EQ(ER_SUCCESS, table.register_price_definition(definition));
    FT_ASSERT_EQ(ER_SUCCESS, table.register_rarity_band(band));
    FT_ASSERT_EQ(ER_SUCCESS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(ER_SUCCESS, table.register_currency_rate(rate));
    return (1);
}

FT_TEST(test_economy_table_copy_constructor, "copy constructor duplicates records and error state")
{
    ft_economy_table source;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_vendor_profile(77, vendor));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());

    ft_economy_table copy(source);
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_price_definition(4, price));
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_currency_rate(5, currency));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    return (1);
}

FT_TEST(test_economy_table_copy_assignment, "copy assignment replaces state and error code")
{
    ft_economy_table source;
    ft_economy_table destination;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    register_all_records(destination);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_price_definition(999, price));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    destination = source;

    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_price_definition(4, price));
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_currency_rate(5, currency));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    return (1);
}

FT_TEST(test_economy_table_move_semantics, "move constructor and assignment transfer state")
{
    ft_economy_table source;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_vendor_profile(77, vendor));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());

    ft_economy_table moved(ft_move(source));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_price_definition(4, price));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_currency_rate(5, currency));
    FT_ASSERT(source.get_price_definitions().empty());
    FT_ASSERT(source.get_rarity_bands().empty());
    FT_ASSERT(source.get_vendor_profiles().empty());
    FT_ASSERT(source.get_currency_rates().empty());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());

    ft_economy_table reassigned;
    register_all_records(reassigned);
    reassigned = ft_move(moved);
    FT_ASSERT_EQ(ER_SUCCESS, reassigned.fetch_price_definition(4, price));
    FT_ASSERT(moved.get_price_definitions().empty());
    FT_ASSERT(moved.get_rarity_bands().empty());
    FT_ASSERT(moved.get_vendor_profiles().empty());
    FT_ASSERT(moved.get_currency_rates().empty());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}
