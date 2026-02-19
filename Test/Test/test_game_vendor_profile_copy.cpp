#include "../test_internal.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vendor_profile_constructor_and_setters, "ft_vendor_profile stores constructor values and setters update them")
{
    ft_vendor_profile profile(3, 1.20, 0.88, 0.05);

    FT_ASSERT_EQ(3, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.20, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.88, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.05, profile.get_tax_rate());

    profile.set_vendor_id(5);
    profile.set_buy_markup(1.35);
    profile.set_sell_multiplier(0.72);
    profile.set_tax_rate(0.10);

    FT_ASSERT_EQ(5, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.35, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.72, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.10, profile.get_tax_rate());
    return (1);
}

FT_TEST(test_vendor_profile_initialize_copy_matches_source, "ft_vendor_profile initialize(copy) duplicates fields")
{
    ft_vendor_profile original(8, 1.45, 0.65, 0.12);
    ft_vendor_profile clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(original));

    FT_ASSERT_EQ(8, clone.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(1.45, clone.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(0.65, clone.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(0.12, clone.get_tax_rate());
    return (1);
}
