#include "../../Game/ft_rarity_band.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_rarity_band_default_values, "Game: rarity band defaults to base values")
{
    ft_rarity_band rarity_band;

    FT_ASSERT_EQ(0, rarity_band.get_rarity());
    FT_ASSERT_EQ(1.0, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_parameterized_constructor_sets_fields, "Game: parameterized rarity band stores provided values")
{
    ft_rarity_band rarity_band(3, 2.5);

    FT_ASSERT_EQ(3, rarity_band.get_rarity());
    FT_ASSERT_EQ(2.5, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_set_rarity_overwrites_previous, "Game: set_rarity updates stored rarity")
{
    ft_rarity_band rarity_band;

    rarity_band.set_rarity(5);
    FT_ASSERT_EQ(5, rarity_band.get_rarity());
    rarity_band.set_rarity(8);
    FT_ASSERT_EQ(8, rarity_band.get_rarity());
    return (1);
}

FT_TEST(test_rarity_band_set_value_multiplier_overwrites_previous, "Game: set_value_multiplier updates multiplier")
{
    ft_rarity_band rarity_band;

    rarity_band.set_value_multiplier(1.5);
    FT_ASSERT_EQ(1.5, rarity_band.get_value_multiplier());
    rarity_band.set_value_multiplier(0.75);
    FT_ASSERT_EQ(0.75, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_set_rarity_sets_errno_success, "Game: set_rarity sets errno to success on update")
{
    ft_rarity_band rarity_band;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    rarity_band.set_rarity(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(9, rarity_band.get_rarity());
    return (1);
}

FT_TEST(test_rarity_band_set_multiplier_sets_errno_success, "Game: set_value_multiplier sets errno to success on update")
{
    ft_rarity_band rarity_band;

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    rarity_band.set_value_multiplier(3.25);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(3.25, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_get_rarity_sets_errno_success, "Game: get_rarity clears errno to success after read")
{
    ft_rarity_band rarity_band;

    rarity_band.set_rarity(4);
    ft_errno = FT_ERR_INVALID_POINTER;
    FT_ASSERT_EQ(4, rarity_band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_rarity_band_get_multiplier_sets_errno_success, "Game: get_value_multiplier clears errno to success after read")
{
    ft_rarity_band rarity_band;

    rarity_band.set_value_multiplier(6.5);
    ft_errno = FT_ERR_GAME_INVALID_MOVE;
    FT_ASSERT_EQ(6.5, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_rarity_band_get_error_reports_current_state, "Game: get_error returns stored error code")
{
    ft_rarity_band rarity_band;

    rarity_band.set_rarity(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_get_error_str_reports_success, "Game: get_error_str describes success and resets errno")
{
    ft_rarity_band rarity_band;

    rarity_band.set_value_multiplier(1.1);
    ft_errno = FT_ERR_NOT_FOUND;
    FT_ASSERT_STR_EQ(ft_strerror(FT_ERR_SUCCESSS), rarity_band.get_error_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_rarity_band_copy_constructor_clones_values, "Game: copy constructor duplicates rarity band values")
{
    ft_rarity_band original(7, 4.0);

    ft_rarity_band copy(original);
    FT_ASSERT_EQ(7, copy.get_rarity());
    FT_ASSERT_EQ(4.0, copy.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(7, original.get_rarity());
    FT_ASSERT_EQ(4.0, original.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_copy_assignment_replaces_destination, "Game: copy assignment overwrites destination values")
{
    ft_rarity_band source(6, 2.0);
    ft_rarity_band destination(1, 0.5);

    destination = source;
    FT_ASSERT_EQ(6, destination.get_rarity());
    FT_ASSERT_EQ(2.0, destination.get_value_multiplier());
    FT_ASSERT_EQ(1, source.get_rarity());
    FT_ASSERT_EQ(0.5, source.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_move_constructor_transfers_values, "Game: move constructor transfers values and resets origin")
{
    ft_rarity_band original(9, 5.5);

    ft_rarity_band moved(ft_move(original));
    FT_ASSERT_EQ(9, moved.get_rarity());
    FT_ASSERT_EQ(5.5, moved.get_value_multiplier());
    FT_ASSERT_EQ(0, original.get_rarity());
    FT_ASSERT_EQ(0.0, original.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_move_assignment_transfers_and_resets, "Game: move assignment transfers values and clears origin")
{
    ft_rarity_band source(12, 7.25);
    ft_rarity_band destination(3, 1.0);

    destination = ft_move(source);
    FT_ASSERT_EQ(12, destination.get_rarity());
    FT_ASSERT_EQ(7.25, destination.get_value_multiplier());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_EQ(0.0, source.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_self_copy_assignment_noop, "Game: self copy assignment leaves rarity band unchanged")
{
    ft_rarity_band rarity_band(10, 1.8);

    rarity_band = rarity_band;
    FT_ASSERT_EQ(10, rarity_band.get_rarity());
    FT_ASSERT_EQ(1.8, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_self_move_assignment_noop, "Game: self move assignment leaves rarity band unchanged")
{
    ft_rarity_band rarity_band(14, 9.9);

    rarity_band = ft_move(rarity_band);
    FT_ASSERT_EQ(14, rarity_band.get_rarity());
    FT_ASSERT_EQ(9.9, rarity_band.get_value_multiplier());
    return (1);
}

FT_TEST(test_rarity_band_set_rarity_single_global_error,
    "set_rarity records exactly one global error entry")
{
    ft_rarity_band band;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(band.set_rarity(8));
    return (1);
}
