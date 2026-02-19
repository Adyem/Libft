#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_region_definition_field_accessors, "Region definition getters reflect stored values")
{
    ft_region_definition region(12, ft_string("Verdant Vale"),
            ft_string("Lush forests with hidden ruins."), 18);

    FT_ASSERT_EQ(12, region.get_region_id());
    FT_ASSERT_EQ(ft_string("Verdant Vale"), region.get_name());
    FT_ASSERT_EQ(ft_string("Lush forests with hidden ruins."), region.get_description());
    FT_ASSERT_EQ(18, region.get_recommended_level());
    return (1);
}

FT_TEST(test_vendor_profile_initialize_copy, "Vendor profile initialize(copy) duplicates marketplace configuration")
{
    ft_vendor_profile source(3, 1.2, 0.8, 0.05);
    ft_vendor_profile clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(3, clone.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.2, clone.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.8, clone.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.05, clone.get_tax_rate());
    return (1);
}

FT_TEST(test_rarity_band_setters_store_values, "Rarity band setters override stored rarity and multiplier")
{
    ft_rarity_band band;

    band.set_rarity(2);
    FT_ASSERT_EQ(2, band.get_rarity());
    band.set_value_multiplier(2.25);
    FT_ASSERT_EQ(2.25, band.get_value_multiplier());
    return (1);
}
