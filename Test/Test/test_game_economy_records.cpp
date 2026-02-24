#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int assert_price_values(const ft_price_definition &definition, int item_id, int rarity,
        int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    return (1);
}

static int assert_rarity_values(const ft_rarity_band &band, int rarity,
        double multiplier)
{
    FT_ASSERT_EQ(rarity, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(multiplier, band.get_value_multiplier());
    return (1);
}

static int assert_rate_values(const ft_currency_rate &rate, int currency_id,
        double base_rate, int precision)
{
    FT_ASSERT_EQ(currency_id, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(base_rate, rate.get_rate_to_base());
    FT_ASSERT_EQ(precision, rate.get_display_precision());
    return (1);
}

FT_TEST(test_price_definition_initialize_copy_matches_source, "price definition initialize(copy) matches the source")
{
    ft_price_definition original;
    ft_price_definition copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(7, 2, 120, 80, 200));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(original));

    assert_price_values(copy, 7, 2, 120, 80, 200);
    assert_price_values(original, 7, 2, 120, 80, 200);
    return (1);
}

FT_TEST(test_price_definition_setters_overwrite_fields, "price definition setters overwrite previous values")
{
    ft_price_definition definition;

    definition.set_item_id(5);
    definition.set_rarity(4);
    definition.set_base_value(600);
    definition.set_minimum_value(250);
    definition.set_maximum_value(750);

    assert_price_values(definition, 5, 4, 600, 250, 750);
    return (1);
}

FT_TEST(test_rarity_band_initialize_copy_matches_source, "rarity band initialize(copy) duplicates the original")
{
    ft_rarity_band band;
    ft_rarity_band copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(4, 1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(band));

    assert_rarity_values(copy, 4, 1.5);
    assert_rarity_values(band, 4, 1.5);
    return (1);
}

FT_TEST(test_currency_rate_initialize_copy_matches_source, "currency rate initialize(copy) duplicates the original")
{
    ft_currency_rate rate;
    ft_currency_rate copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(5, 0.5, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(rate));

    assert_rate_values(copy, 5, 0.5, 3);
    assert_rate_values(rate, 5, 0.5, 3);
    return (1);
}
