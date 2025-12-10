#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

static int assert_price_values(const ft_price_definition &definition, int item_id, int rarity,
        int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_copy_and_move, "copy and move price definitions")
{
    ft_price_definition original(7, 2, 120, 80, 200);
    ft_price_definition copy(original);
    ft_price_definition moved(ft_move(original));

    assert_price_values(copy, 7, 2, 120, 80, 200);
    assert_price_values(moved, 7, 2, 120, 80, 200);
    FT_ASSERT_EQ(0, original.get_item_id());
    FT_ASSERT_EQ(0, original.get_rarity());
    FT_ASSERT_EQ(0, original.get_base_value());
    FT_ASSERT_EQ(0, original.get_minimum_value());
    FT_ASSERT_EQ(0, original.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}

FT_TEST(test_rarity_band_copy_and_move, "copy and move rarity bands")
{
    ft_rarity_band band(4, 1.5);
    ft_rarity_band copy(band);
    ft_rarity_band moved(ft_move(band));

    FT_ASSERT_EQ(4, copy.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.5, copy.get_value_multiplier());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(4, moved.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.5, moved.get_value_multiplier());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved.get_error());
    FT_ASSERT_EQ(0, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(0.0, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, band.get_error());
    return (1);
}

FT_TEST(test_currency_rate_copy_and_move, "copy and move currency rates")
{
    ft_currency_rate rate(5, 0.5, 3);
    ft_currency_rate copy(rate);
    ft_currency_rate moved(ft_move(rate));

    FT_ASSERT_EQ(5, copy.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, copy.get_rate_to_base());
    FT_ASSERT_EQ(3, copy.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(5, moved.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, moved.get_rate_to_base());
    FT_ASSERT_EQ(3, moved.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved.get_error());
    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(0, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}
