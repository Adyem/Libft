#include "../../Game/ft_vendor_profile.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_vendor_profile_copy_constructor, "copy constructor duplicates vendor attributes")
{
    ft_vendor_profile original(42, 1.15, 0.85, 0.07);
    ft_vendor_profile copy(original);

    original.set_vendor_id(99);
    original.set_buy_markup(1.05);
    original.set_sell_multiplier(0.95);
    original.set_tax_rate(0.09);

    FT_ASSERT_EQ(42, copy.get_vendor_id());
    FT_ASSERT_EQ(1.15, copy.get_buy_markup());
    FT_ASSERT_EQ(0.85, copy.get_sell_multiplier());
    FT_ASSERT_EQ(0.07, copy.get_tax_rate());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    return (1);
}

FT_TEST(test_vendor_profile_copy_assignment, "copy assignment overwrites vendor state safely")
{
    ft_vendor_profile source(7, 1.25, 0.75, 0.06);
    ft_vendor_profile destination(13, 1.05, 0.90, 0.03);

    destination = source;
    source.set_vendor_id(21);
    source.set_buy_markup(1.40);
    source.set_sell_multiplier(0.65);
    source.set_tax_rate(0.10);

    FT_ASSERT_EQ(7, destination.get_vendor_id());
    FT_ASSERT_EQ(1.25, destination.get_buy_markup());
    FT_ASSERT_EQ(0.75, destination.get_sell_multiplier());
    FT_ASSERT_EQ(0.06, destination.get_tax_rate());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}
