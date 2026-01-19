#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_price_definition_errno_sets_to_success_during_operations, "Price definition getters and setters set errno to success")
{
    ft_price_definition definition(3, 4, 120, 60, 240);

    ft_errno = FT_ERR_INVALID_STATE;
    definition.set_item_id(7);
    definition.set_rarity(6);
    definition.set_base_value(300);
    definition.set_minimum_value(80);
    definition.set_maximum_value(500);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_ALREADY_EXISTS;
    FT_ASSERT_EQ(7, definition.get_item_id());
    FT_ASSERT_EQ(6, definition.get_rarity());
    FT_ASSERT_EQ(300, definition.get_base_value());
    FT_ASSERT_EQ(80, definition.get_minimum_value());
    FT_ASSERT_EQ(500, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, definition.get_error());
    return (1);
}


FT_TEST(test_price_definition_copy_constructor_sets_errno_success, "Price definition copy constructor sets errno to success")
{
    ft_price_definition original(9, 2, 150, 90, 210);

    ft_errno = FT_ERR_CONFIGURATION;
    ft_price_definition copy(original);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(9, copy.get_item_id());
    FT_ASSERT_EQ(2, copy.get_rarity());
    FT_ASSERT_EQ(150, copy.get_base_value());
    FT_ASSERT_EQ(90, copy.get_minimum_value());
    FT_ASSERT_EQ(210, copy.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, original.get_error());
    return (1);
}


FT_TEST(test_price_definition_move_assignment_sets_errno_and_clears_source, "Price definition move assignment sets errno to success and clears source")
{
    ft_price_definition source(12, 7, 640, 120, 980);
    ft_price_definition destination;

    ft_errno = FT_ERR_INTERNAL;
    destination = ft_move(source);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(12, destination.get_item_id());
    FT_ASSERT_EQ(7, destination.get_rarity());
    FT_ASSERT_EQ(640, destination.get_base_value());
    FT_ASSERT_EQ(120, destination.get_minimum_value());
    FT_ASSERT_EQ(980, destination.get_maximum_value());
    FT_ASSERT_EQ(0, source.get_item_id());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_EQ(0, source.get_base_value());
    FT_ASSERT_EQ(0, source.get_minimum_value());
    FT_ASSERT_EQ(0, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());
    return (1);
}


FT_TEST(test_price_definition_get_error_str_reports_success_message, "Price definition get_error_str returns success message")
{
    ft_price_definition definition;

    FT_ASSERT_STR_EQ(ft_strerror(FT_ERR_SUCCESSS), definition.get_error_str());
    return (1);
}


FT_TEST(test_currency_rate_errno_sets_to_success_during_operations, "Currency rate getters and setters set errno to success")
{
    ft_currency_rate rate(5, 2.25, 3);

    ft_errno = FT_ERR_PRIORITY_QUEUE_EMPTY;
    rate.set_currency_id(11);
    rate.set_rate_to_base(0.85);
    rate.set_display_precision(6);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_ALREADY_INITIALIZED;
    FT_ASSERT_EQ(11, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.85, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_move_constructor_sets_errno_and_resets_source, "Currency rate move constructor sets errno to success and resets source")
{
    ft_currency_rate original(18, 4.5, 1);

    ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
    ft_currency_rate moved(ft_move(original));

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(18, moved.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.5, moved.get_rate_to_base());
    FT_ASSERT_EQ(1, moved.get_display_precision());
    FT_ASSERT_EQ(0, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, original.get_rate_to_base());
    FT_ASSERT_EQ(0, original.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, original.get_error());
    return (1);
}


FT_TEST(test_currency_rate_get_error_str_reports_success_message, "Currency rate get_error_str returns success message")
{
    ft_currency_rate rate;

    FT_ASSERT_STR_EQ(ft_strerror(FT_ERR_SUCCESSS), rate.get_error_str());
    return (1);
}


FT_TEST(test_rarity_band_errno_sets_to_success_during_operations, "Rarity band getters and setters set errno to success")
{
    ft_rarity_band band(6, 3.25);

    ft_errno = FT_ERR_HTTP_PROTOCOL_MISMATCH;
    band.set_rarity(9);
    band.set_value_multiplier(1.1);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SOCKET_LISTEN_FAILED;
    FT_ASSERT_EQ(9, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.1, band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, band.get_error());
    return (1);
}


FT_TEST(test_rarity_band_move_assignment_sets_errno_and_resets_source, "Rarity band move assignment sets errno to success and resets source")
{
    ft_rarity_band source(14, 2.8);
    ft_rarity_band destination(3, 0.9);

    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    destination = ft_move(source);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(14, destination.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.8, destination.get_value_multiplier());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_currency_rate_set_currency_id_single_global_error,
    "set_currency_id records exactly one error stack entry")
{
    ft_currency_rate rate;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(rate.set_currency_id(7));
    return (1);
}

FT_TEST(test_rarity_band_set_multiplier_single_global_error,
    "set_value_multiplier records exactly one global error entry")
{
    ft_rarity_band band;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(band.set_value_multiplier(3.45));
    return (1);
}


FT_TEST(test_rarity_band_get_error_str_reports_success_message, "Rarity band get_error_str returns success message")
{
    ft_rarity_band band;

    FT_ASSERT_STR_EQ(ft_strerror(FT_ERR_SUCCESSS), band.get_error_str());
    return (1);
}
