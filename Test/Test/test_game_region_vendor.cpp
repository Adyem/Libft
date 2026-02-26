#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_region_definition_field_accessors, "Region definition getters reflect stored values")
{
    ft_region_definition region;

    ft_string verdant_vale;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, verdant_vale.initialize("Verdant Vale"));
    ft_string lush_forests;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lush_forests.initialize("Lush forests with hidden ruins."));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(12, verdant_vale,
        lush_forests, 18));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(12, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("Verdant Vale", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("Lush forests with hidden ruins.", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(18, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_initialize_copy, "Vendor profile initialize(copy) duplicates marketplace configuration")
{
    ft_vendor_profile source;
    ft_vendor_profile clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 1.2, 0.8, 0.05));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(3, clone.get_vendor_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_DOUBLE_EQ(1.2, clone.get_buy_markup());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_DOUBLE_EQ(0.8, clone.get_sell_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_DOUBLE_EQ(0.05, clone.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_store_values, "Rarity band setters override stored rarity and multiplier")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    band.set_rarity(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(2, band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    band.set_value_multiplier(2.25);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(2.25, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}
