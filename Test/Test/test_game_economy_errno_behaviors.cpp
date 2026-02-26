#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_definition_setters_preserve_values, "Price definition setters persist the configured values")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(3, 4, 120, 60, 240));

    definition.set_item_id(7);
    definition.set_rarity(6);
    definition.set_base_value(300);
    definition.set_minimum_value(80);
    definition.set_maximum_value(500);

    FT_ASSERT_EQ(7, definition.get_item_id());
    FT_ASSERT_EQ(6, definition.get_rarity());
    FT_ASSERT_EQ(300, definition.get_base_value());
    FT_ASSERT_EQ(80, definition.get_minimum_value());
    FT_ASSERT_EQ(500, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_initialize_copy_preserves_values, "Price definition initialize(copy) duplicates the source state")
{
    ft_price_definition source;
    ft_price_definition copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(9, 2, 150, 90, 210));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));

    FT_ASSERT_EQ(9, copy.get_item_id());
    FT_ASSERT_EQ(2, copy.get_rarity());
    FT_ASSERT_EQ(150, copy.get_base_value());
    FT_ASSERT_EQ(90, copy.get_minimum_value());
    FT_ASSERT_EQ(210, copy.get_maximum_value());
    FT_ASSERT_EQ(9, source.get_item_id());
    FT_ASSERT_EQ(2, source.get_rarity());
    FT_ASSERT_EQ(150, source.get_base_value());
    FT_ASSERT_EQ(90, source.get_minimum_value());
    FT_ASSERT_EQ(210, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    return (1);
}

FT_TEST(test_currency_rate_setters_preserve_values, "Currency rate setters persist their values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(5, 2.25, 3));

    rate.set_currency_id(11);
    rate.set_rate_to_base(0.85);
    rate.set_display_precision(6);

    FT_ASSERT_EQ(11, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.85, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_initialize_copy_preserves_values, "Currency rate initialize(copy) duplicates source state")
{
    ft_currency_rate source;
    ft_currency_rate clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(18, 4.5, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));

    FT_ASSERT_EQ(18, clone.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.5, clone.get_rate_to_base());
    FT_ASSERT_EQ(1, clone.get_display_precision());
    FT_ASSERT_EQ(18, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.5, source.get_rate_to_base());
    FT_ASSERT_EQ(1, source.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_preserve_values, "Rarity band setters persist their values")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(6, 3.25));

    band.set_rarity(9);
    band.set_value_multiplier(1.1);

    FT_ASSERT_EQ(9, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.1, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_initialize_copy_preserves_values, "Rarity band initialize(copy) duplicates source state")
{
    ft_rarity_band source;
    ft_rarity_band clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(14, 2.8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));

    FT_ASSERT_EQ(14, clone.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.8, clone.get_value_multiplier());
    FT_ASSERT_EQ(14, source.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.8, source.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}
