#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_copy_into_uninitialized_destination_succeeds, "price definition copy initialize succeeds for uninitialized destination")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(15, 4, 420, 200, 900));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(15, destination.get_item_id());
    return (1);
}

FT_TEST(test_price_move_into_uninitialized_destination_succeeds, "price definition move initialize succeeds for uninitialized destination")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(16, 2, 320, 150, 600));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(16, destination.get_item_id());
    return (1);
}

FT_TEST(test_price_copy_into_destroyed_destination_succeeds, "price definition copy initialize succeeds for destroyed destination")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(17, 3, 500, 300, 950));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(17, destination.get_item_id());
    return (1);
}

FT_TEST(test_price_move_into_destroyed_destination_succeeds, "price definition move initialize succeeds for destroyed destination")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(18, 1, 140, 70, 300));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(18, destination.get_item_id());
    return (1);
}

FT_TEST(test_rarity_copy_into_uninitialized_destination_succeeds, "rarity band copy initialize succeeds for uninitialized destination")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(9, 1.9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(9, destination.get_rarity());
    return (1);
}

FT_TEST(test_rarity_move_into_uninitialized_destination_succeeds, "rarity band move initialize succeeds for uninitialized destination")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(10, 2.2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(10, destination.get_rarity());
    return (1);
}

FT_TEST(test_rarity_copy_into_destroyed_destination_succeeds, "rarity band copy initialize succeeds for destroyed destination")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(11, 1.3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(11, destination.get_rarity());
    return (1);
}

FT_TEST(test_rarity_move_into_destroyed_destination_succeeds, "rarity band move initialize succeeds for destroyed destination")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(12, 0.9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(12, destination.get_rarity());
    return (1);
}

FT_TEST(test_vendor_copy_into_uninitialized_destination_succeeds, "vendor profile copy initialize succeeds for uninitialized destination")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(30, 1.2, 0.7, 0.06));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(30, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_move_into_uninitialized_destination_succeeds, "vendor profile move initialize succeeds for uninitialized destination")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(31, 1.1, 0.8, 0.04));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(31, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_copy_into_destroyed_destination_succeeds, "vendor profile copy initialize succeeds for destroyed destination")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(32, 1.4, 0.6, 0.08));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(32, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_move_into_destroyed_destination_succeeds, "vendor profile move initialize succeeds for destroyed destination")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(33, 1.05, 0.85, 0.03));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(33, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_currency_copy_into_uninitialized_destination_succeeds, "currency rate copy initialize succeeds for uninitialized destination")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(40, 1.75, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(40, destination.get_currency_id());
    return (1);
}

FT_TEST(test_currency_move_into_uninitialized_destination_succeeds, "currency rate move initialize succeeds for uninitialized destination")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(41, 0.66, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(41, destination.get_currency_id());
    return (1);
}

FT_TEST(test_currency_copy_into_destroyed_destination_succeeds, "currency rate copy initialize succeeds for destroyed destination")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(42, 2.2, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(42, destination.get_currency_id());
    return (1);
}

FT_TEST(test_currency_move_into_destroyed_destination_succeeds, "currency rate move initialize succeeds for destroyed destination")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(43, 3.1, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(43, destination.get_currency_id());
    return (1);
}

FT_TEST(test_table_copy_into_uninitialized_destination_succeeds, "economy table copy initialize succeeds for uninitialized destination")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    return (1);
}

FT_TEST(test_table_move_into_uninitialized_destination_succeeds, "economy table move initialize succeeds for uninitialized destination")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    return (1);
}

FT_TEST(test_table_copy_into_destroyed_destination_succeeds, "economy table copy initialize succeeds for destroyed destination")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    return (1);
}

FT_TEST(test_table_move_into_destroyed_destination_succeeds, "economy table move initialize succeeds for destroyed destination")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    return (1);
}
