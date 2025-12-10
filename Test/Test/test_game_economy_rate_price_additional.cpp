#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Template/move.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_currency_rate_default_initialization_success, "Currency rate defaults to base values and success")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_parameterized_constructor_sets_fields, "Currency rate parameterized constructor stores provided values")
{
    ft_currency_rate rate(4, 0.5, 6);

    FT_ASSERT_EQ(4, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_rate_copy_constructor_resets_errno_and_keeps_source, "Currency rate copy constructor resets errno to success and leaves source intact")
{
    ft_currency_rate original(8, 1.35, 5);

    ft_errno = FT_ERR_GAME_INVALID_MOVE;
    ft_currency_rate copy(original);

    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(8, copy.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.35, copy.get_rate_to_base());
    FT_ASSERT_EQ(5, copy.get_display_precision());
    FT_ASSERT_EQ(8, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.35, original.get_rate_to_base());
    FT_ASSERT_EQ(5, original.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}

FT_TEST(test_currency_rate_move_assignment_transfers_values_and_errno_success, "Currency rate move assignment resets errno to success while clearing source")
{
    ft_currency_rate source(11, 3.5, 1);
    ft_currency_rate destination;

    ft_errno = FT_ERR_SOCKET_RECEIVE_FAILED;
    destination = ft_move(source);

    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(11, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.5, destination.get_rate_to_base());
    FT_ASSERT_EQ(1, destination.get_display_precision());
    FT_ASSERT_EQ(0, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_rate_to_base());
    FT_ASSERT_EQ(0, source.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_currency_rate_setters_update_fields, "Currency rate setters replace stored identifiers and precision")
{
    ft_currency_rate rate;

    rate.set_currency_id(13);
    rate.set_rate_to_base(4.25);
    rate.set_display_precision(0);

    FT_ASSERT_EQ(13, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.25, rate.get_rate_to_base());
    FT_ASSERT_EQ(0, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}

FT_TEST(test_price_definition_default_initialization_success, "Price definition defaults to zeroed fields and success")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_parameterized_constructor_sets_fields, "Price definition constructor stores provided values")
{
    ft_price_definition definition(10, 3, 700, 450, 900);

    FT_ASSERT_EQ(10, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(700, definition.get_base_value());
    FT_ASSERT_EQ(450, definition.get_minimum_value());
    FT_ASSERT_EQ(900, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_copy_constructor_resets_errno_and_retains_source, "Price definition copy constructor resets errno to success and retains source values")
{
    ft_price_definition original(12, 5, 250, 100, 400);

    ft_errno = FT_ERR_ALREADY_INITIALIZED;
    ft_price_definition copy(original);

    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(12, copy.get_item_id());
    FT_ASSERT_EQ(5, copy.get_rarity());
    FT_ASSERT_EQ(250, copy.get_base_value());
    FT_ASSERT_EQ(100, copy.get_minimum_value());
    FT_ASSERT_EQ(400, copy.get_maximum_value());
    FT_ASSERT_EQ(12, original.get_item_id());
    FT_ASSERT_EQ(5, original.get_rarity());
    FT_ASSERT_EQ(250, original.get_base_value());
    FT_ASSERT_EQ(100, original.get_minimum_value());
    FT_ASSERT_EQ(400, original.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}

FT_TEST(test_price_definition_move_assignment_resets_errno_and_resets_source, "Price definition move assignment resets errno to success and clears origin")
{
    ft_price_definition source(14, 6, 950, 600, 1200);
    ft_price_definition destination;

    ft_errno = FT_ERR_SOCKET_LISTEN_FAILED;
    destination = ft_move(source);

    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(14, destination.get_item_id());
    FT_ASSERT_EQ(6, destination.get_rarity());
    FT_ASSERT_EQ(950, destination.get_base_value());
    FT_ASSERT_EQ(600, destination.get_minimum_value());
    FT_ASSERT_EQ(1200, destination.get_maximum_value());
    FT_ASSERT_EQ(0, source.get_item_id());
    FT_ASSERT_EQ(0, source.get_rarity());
    FT_ASSERT_EQ(0, source.get_base_value());
    FT_ASSERT_EQ(0, source.get_minimum_value());
    FT_ASSERT_EQ(0, source.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_price_definition_setters_update_all_fields_and_errno, "Price definition setters refresh identifiers, value bounds, and reset errno")
{
    ft_price_definition definition;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    definition.set_item_id(21);
    definition.set_rarity(7);
    definition.set_base_value(1500);
    definition.set_minimum_value(1200);
    definition.set_maximum_value(1800);

    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(21, definition.get_item_id());
    FT_ASSERT_EQ(7, definition.get_rarity());
    FT_ASSERT_EQ(1500, definition.get_base_value());
    FT_ASSERT_EQ(1200, definition.get_minimum_value());
    FT_ASSERT_EQ(1800, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, definition.get_error());
    return (1);
}

FT_TEST(test_price_definition_error_string_reports_success, "Price definition error string reports success by default")
{
    ft_price_definition definition;

    FT_ASSERT_STR_EQ(ft_strerror(FT_ER_SUCCESSS), definition.get_error_str());
    return (1);
}
