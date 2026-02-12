#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_region_definition_setters_update_fields, "Game: region definition setters update stored values")
{
    ft_region_definition region;
    ft_string region_name("Verdant Vale");
    ft_string region_description("Lush forests with hidden ruins.");

    region.set_region_id(42);
    region.set_name(region_name);
    region.set_description(region_description);
    region.set_recommended_level(18);

    FT_ASSERT_EQ(42, region.get_region_id());
    FT_ASSERT_STR_EQ(region_name.c_str(), region.get_name().c_str());
    FT_ASSERT_STR_EQ(region_description.c_str(), region.get_description().c_str());
    FT_ASSERT_EQ(18, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_region_definition_move_clears_source, "Game: moving region definitions clears source state")
{
    ft_region_definition original(7, ft_string("Frostfall Pass"), ft_string("Snowy cliffs"), 12);
    ft_region_definition moved(ft_move(original));

    FT_ASSERT_EQ(7, moved.get_region_id());
    FT_ASSERT_STR_EQ("Frostfall Pass", moved.get_name().c_str());
    FT_ASSERT_STR_EQ("Snowy cliffs", moved.get_description().c_str());
    FT_ASSERT_EQ(12, moved.get_recommended_level());
    FT_ASSERT_EQ(0, original.get_region_id());
    FT_ASSERT_STR_EQ("", original.get_name().c_str());
    FT_ASSERT_STR_EQ("", original.get_description().c_str());
    FT_ASSERT_EQ(0, original.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_copy_preserves_values, "Game: vendor profile copy keeps pricing configuration")
{
    ft_vendor_profile vendor(3, 1.2, 0.8, 0.05);
    ft_vendor_profile copy(vendor);

    FT_ASSERT_EQ(3, copy.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.2, copy.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.8, copy.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.05, copy.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_move_resets_original, "Game: vendor profile move transfers values and resets source")
{
    ft_vendor_profile vendor(9, 1.35, 0.65, 0.12);
    ft_vendor_profile moved(ft_move(vendor));

    FT_ASSERT_EQ(9, moved.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.35, moved.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.65, moved.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.12, moved.get_tax_rate());
    FT_ASSERT_EQ(0, vendor.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.0, vendor.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(1.0, vendor.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.0, vendor.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_overwrite_previous_values, "Game: rarity band setters override stored multiplier")
{
    ft_rarity_band band;

    band.set_rarity(2);
    band.set_value_multiplier(2.25);

    FT_ASSERT_EQ(2, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.25, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}
