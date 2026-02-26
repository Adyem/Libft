#include "../test_internal.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vendor_profile_initialize_values, "ft_vendor_profile initialize sets fields")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(4, 1.20, 0.80, 0.05));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(4, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.20, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.80, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.05, profile.get_tax_rate());
    return (1);
}

FT_TEST(test_vendor_profile_setters_update_fields, "ft_vendor_profile setters override stored values")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    profile.set_vendor_id(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_buy_markup(1.35);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_sell_multiplier(0.65);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_tax_rate(0.12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());

    FT_ASSERT_EQ(9, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.35, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.65, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.12, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_set_negative_id_reports_error, "ft_vendor_profile rejects negative vendor IDs")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    profile.set_vendor_id(-2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_initialize_copy, "ft_vendor_profile initialize(copy) duplicates fields")
{
    ft_vendor_profile source;
    ft_vendor_profile clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, 1.45, 0.70, 0.10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(7, clone.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.45, clone.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.70, clone.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.10, clone.get_tax_rate());
    return (1);
}
