#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_lifecycle_initialize_copy_from_initialized_source, "price definition initialize(copy) works with initialized source")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(9, 3, 300, 120, 600));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(9, destination.get_item_id());
    return (1);
}

FT_TEST(test_price_lifecycle_initialize_move_from_initialized_source, "price definition initialize(move) works with initialized source")
{
    ft_price_definition source;
    ft_price_definition destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(12, 4, 500, 200, 900));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(12, destination.get_item_id());
    return (1);
}

FT_TEST(test_price_lifecycle_initialize_copy_self_noop, "price definition initialize(copy) self is no-op")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(4, 1, 80, 40, 120));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(definition));
    FT_ASSERT_EQ(4, definition.get_item_id());
    return (1);
}

FT_TEST(test_price_lifecycle_initialize_move_self_noop, "price definition initialize(move) self is no-op")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(7, 2, 150, 75, 220));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(ft_move(definition)));
    FT_ASSERT_EQ(7, definition.get_item_id());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_copy_from_initialized_source, "rarity band initialize(copy) works with initialized source")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(6, 1.8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(6, destination.get_rarity());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_move_from_initialized_source, "rarity band initialize(move) works with initialized source")
{
    ft_rarity_band source;
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 2.1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(3, destination.get_rarity());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_copy_self_noop, "rarity band initialize(copy) self is no-op")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(8, 1.4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(band));
    FT_ASSERT_EQ(8, band.get_rarity());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_move_self_noop, "rarity band initialize(move) self is no-op")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(5, 1.6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(ft_move(band)));
    FT_ASSERT_EQ(5, band.get_rarity());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_copy_from_initialized_source, "vendor profile initialize(copy) works with initialized source")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(11, 1.3, 0.6, 0.08));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(11, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_move_from_initialized_source, "vendor profile initialize(move) works with initialized source")
{
    ft_vendor_profile source;
    ft_vendor_profile destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(13, 1.2, 0.7, 0.04));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(13, destination.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_copy_self_noop, "vendor profile initialize(copy) self is no-op")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(21, 1.1, 0.8, 0.03));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(profile));
    FT_ASSERT_EQ(21, profile.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_move_self_noop, "vendor profile initialize(move) self is no-op")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(22, 1.05, 0.82, 0.02));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(ft_move(profile)));
    FT_ASSERT_EQ(22, profile.get_vendor_id());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_copy_from_initialized_source, "currency rate initialize(copy) works with initialized source")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(2, 1.5, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(2, destination.get_currency_id());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_move_from_initialized_source, "currency rate initialize(move) works with initialized source")
{
    ft_currency_rate source;
    ft_currency_rate destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4, 2.0, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(4, destination.get_currency_id());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_copy_self_noop, "currency rate initialize(copy) self is no-op")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(6, 3.3, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(rate));
    FT_ASSERT_EQ(6, rate.get_currency_id());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_move_self_noop, "currency rate initialize(move) self is no-op")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(9, 0.9, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(ft_move(rate)));
    FT_ASSERT_EQ(9, rate.get_currency_id());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_copy_from_initialized_source, "economy table initialize(copy) works with initialized source")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_move_from_initialized_source, "economy table initialize(move) works with initialized source")
{
    ft_economy_table source;
    ft_economy_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_copy_self_noop, "economy table initialize(copy) self is no-op")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize(table));
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_move_self_noop, "economy table initialize(move) self is no-op")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize(ft_move(table)));
    return (1);
}
