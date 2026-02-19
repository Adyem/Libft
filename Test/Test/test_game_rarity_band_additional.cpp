#include "../test_internal.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rarity_band_default_values, "Game: rarity band defaults to base values")
{
    ft_rarity_band rarity_band;

    FT_ASSERT_EQ(0, rarity_band.get_rarity());
    FT_ASSERT_EQ(1.0, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_parameterized_constructor_values, "Game: parameterized rarity band stores provided values")
{
    ft_rarity_band rarity_band(3, 2.5);

    FT_ASSERT_EQ(3, rarity_band.get_rarity());
    FT_ASSERT_EQ(2.5, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_setters_overwrite_values, "Game: setters overwrite stored rarity and multiplier")
{
    ft_rarity_band rarity_band;

    rarity_band.set_rarity(5);
    FT_ASSERT_EQ(5, rarity_band.get_rarity());
    rarity_band.set_rarity(8);
    FT_ASSERT_EQ(8, rarity_band.get_rarity());
    rarity_band.set_value_multiplier(1.5);
    FT_ASSERT_EQ(1.5, rarity_band.get_value_multiplier());
    rarity_band.set_value_multiplier(0.75);
    FT_ASSERT_EQ(0.75, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_initialize_copy_matches_source, "Game: initialize(copy) duplicates configured values")
{
    ft_rarity_band source(7, 4.0);
    ft_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));

    FT_ASSERT_EQ(7, destination.get_rarity());
    FT_ASSERT_EQ(4.0, destination.get_value_multiplier());
    FT_ASSERT_EQ(7, source.get_rarity());
    FT_ASSERT_EQ(4.0, source.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_initialize_overwrites_previous_values, "Game: initialize(copy) resets existing values")
{
    ft_rarity_band source(4, 1.25);
    ft_rarity_band destination(2, 0.5);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));

    FT_ASSERT_EQ(4, destination.get_rarity());
    FT_ASSERT_EQ(1.25, destination.get_value_multiplier());
    return (1);
}
