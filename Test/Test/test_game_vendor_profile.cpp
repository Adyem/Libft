#include "../../Game/ft_vendor_profile.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_vendor_profile_defaults, "Vendor profile initializes default pricing fields")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(0, profile.get_vendor_id());
    FT_ASSERT_EQ(1.0, profile.get_buy_markup());
    FT_ASSERT_EQ(1.0, profile.get_sell_multiplier());
    FT_ASSERT_EQ(0.0, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_parameterized_constructor_sets_fields, "Vendor profile parameterized constructor stores values")
{
    ft_vendor_profile profile(8, 1.6, 0.85, 0.12);

    FT_ASSERT_EQ(8, profile.get_vendor_id());
    FT_ASSERT_EQ(1.6, profile.get_buy_markup());
    FT_ASSERT_EQ(0.85, profile.get_sell_multiplier());
    FT_ASSERT_EQ(0.12, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_copy_constructor_clones_fields, "Vendor profile copy constructor duplicates state")
{
    ft_vendor_profile original(5, 2.0, 0.5, 0.07);

    ft_vendor_profile copy(original);
    FT_ASSERT_EQ(5, copy.get_vendor_id());
    FT_ASSERT_EQ(2.0, copy.get_buy_markup());
    FT_ASSERT_EQ(0.5, copy.get_sell_multiplier());
    FT_ASSERT_EQ(0.07, copy.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_move_constructor_transfers_state, "Vendor profile move constructor transfers and clears source")
{
    ft_vendor_profile source(11, 1.1, 1.3, 0.2);

    ft_vendor_profile moved(ft_move(source));
    FT_ASSERT_EQ(11, moved.get_vendor_id());
    FT_ASSERT_EQ(1.1, moved.get_buy_markup());
    FT_ASSERT_EQ(1.3, moved.get_sell_multiplier());
    FT_ASSERT_EQ(0.2, moved.get_tax_rate());
    FT_ASSERT_EQ(0, source.get_vendor_id());
    FT_ASSERT_EQ(1.0, source.get_buy_markup());
    FT_ASSERT_EQ(1.0, source.get_sell_multiplier());
    FT_ASSERT_EQ(0.0, source.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_move_assignment_resets_destination, "Vendor profile move assignment overwrites and clears origin")
{
    ft_vendor_profile source(20, 0.9, 1.4, 0.15);
    ft_vendor_profile destination(3, 1.2, 0.95, 0.05);

    destination = ft_move(source);
    FT_ASSERT_EQ(20, destination.get_vendor_id());
    FT_ASSERT_EQ(0.9, destination.get_buy_markup());
    FT_ASSERT_EQ(1.4, destination.get_sell_multiplier());
    FT_ASSERT_EQ(0.15, destination.get_tax_rate());
    FT_ASSERT_EQ(0, source.get_vendor_id());
    FT_ASSERT_EQ(1.0, source.get_buy_markup());
    FT_ASSERT_EQ(1.0, source.get_sell_multiplier());
    FT_ASSERT_EQ(0.0, source.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_copy_assignment_overwrites_values, "Vendor profile copy assignment duplicates state across instance")
{
    ft_vendor_profile source(14, 1.25, 0.6, 0.18);
    ft_vendor_profile destination(7, 1.75, 1.1, 0.05);

    destination = source;
    FT_ASSERT_EQ(14, destination.get_vendor_id());
    FT_ASSERT_EQ(1.25, destination.get_buy_markup());
    FT_ASSERT_EQ(0.6, destination.get_sell_multiplier());
    FT_ASSERT_EQ(0.18, destination.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_setters_replace_values, "Vendor profile setters update stored pricing details")
{
    ft_vendor_profile profile;

    profile.set_vendor_id(42);
    profile.set_buy_markup(2.5);
    profile.set_sell_multiplier(0.4);
    profile.set_tax_rate(0.08);
    FT_ASSERT_EQ(42, profile.get_vendor_id());
    FT_ASSERT_EQ(2.5, profile.get_buy_markup());
    FT_ASSERT_EQ(0.4, profile.get_sell_multiplier());
    FT_ASSERT_EQ(0.08, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_getters_reset_errno, "Vendor profile getters set errno to success")
{
    ft_vendor_profile profile(3, 1.7, 0.9, 0.03);

    ft_errno = FT_ERR_GAME_GENERAL_ERROR;
    FT_ASSERT_EQ(1.7, profile.get_buy_markup());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(0.9, profile.get_sell_multiplier());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(0.03, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_self_copy_assignment_no_change, "Self copy assignment keeps vendor profile data intact")
{
    ft_vendor_profile profile(9, 1.05, 1.15, 0.09);

    profile = profile;
    FT_ASSERT_EQ(9, profile.get_vendor_id());
    FT_ASSERT_EQ(1.05, profile.get_buy_markup());
    FT_ASSERT_EQ(1.15, profile.get_sell_multiplier());
    FT_ASSERT_EQ(0.09, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}


FT_TEST(test_game_vendor_profile_self_move_assignment_no_change, "Self move assignment preserves vendor profile state")
{
    ft_vendor_profile profile(6, 2.2, 0.7, 0.11);

    profile = ft_move(profile);
    FT_ASSERT_EQ(6, profile.get_vendor_id());
    FT_ASSERT_EQ(2.2, profile.get_buy_markup());
    FT_ASSERT_EQ(0.7, profile.get_sell_multiplier());
    FT_ASSERT_EQ(0.11, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.get_error());
    return (1);
}
