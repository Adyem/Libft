#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_definition_default_initialization, "Price definition defaults to zeroed values with success")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(0, definition.get_item_id());
    FT_ASSERT_EQ(0, definition.get_rarity());
    FT_ASSERT_EQ(0, definition.get_base_value());
    FT_ASSERT_EQ(0, definition.get_minimum_value());
    FT_ASSERT_EQ(0, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}


FT_TEST(test_price_definition_parameterized_initialization, "Price definition stores provided constructor values")
{
    ft_price_definition definition(19, 3, 240, 120, 360);

    FT_ASSERT_EQ(19, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(240, definition.get_base_value());
    FT_ASSERT_EQ(120, definition.get_minimum_value());
    FT_ASSERT_EQ(360, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}


FT_TEST(test_price_definition_setters_update_all_fields, "Price definition setters mutate values and preserve success")
{
    ft_price_definition definition;

    definition.set_item_id(7);
    definition.set_rarity(5);
    definition.set_base_value(810);
    definition.set_minimum_value(200);
    definition.set_maximum_value(950);

    FT_ASSERT_EQ(7, definition.get_item_id());
    FT_ASSERT_EQ(5, definition.get_rarity());
    FT_ASSERT_EQ(810, definition.get_base_value());
    FT_ASSERT_EQ(200, definition.get_minimum_value());
    FT_ASSERT_EQ(950, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}


FT_TEST(test_price_definition_self_copy_assignment_retains_values, "Self copy assignment leaves price definition unchanged")
{
    ft_price_definition definition(22, 4, 150, 90, 400);

    definition = definition;

    FT_ASSERT_EQ(22, definition.get_item_id());
    FT_ASSERT_EQ(4, definition.get_rarity());
    FT_ASSERT_EQ(150, definition.get_base_value());
    FT_ASSERT_EQ(90, definition.get_minimum_value());
    FT_ASSERT_EQ(400, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}


FT_TEST(test_price_definition_self_move_assignment_noop, "Self move assignment short-circuits without altering fields")
{
    ft_price_definition definition(31, 6, 700, 500, 900);

    definition = ft_move(definition);

    FT_ASSERT_EQ(31, definition.get_item_id());
    FT_ASSERT_EQ(6, definition.get_rarity());
    FT_ASSERT_EQ(700, definition.get_base_value());
    FT_ASSERT_EQ(500, definition.get_minimum_value());
    FT_ASSERT_EQ(900, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}


FT_TEST(test_currency_rate_default_initialization, "Currency rate defaults to base currency values and success")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_parameterized_initialization, "Currency rate stores constructor values")
{
    ft_currency_rate rate(17, 2.75, 5);

    FT_ASSERT_EQ(17, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(2.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_setters_update_fields, "Currency rate setters mutate values and remain successful")
{
    ft_currency_rate rate;

    rate.set_currency_id(9);
    rate.set_rate_to_base(0.65);
    rate.set_display_precision(6);

    FT_ASSERT_EQ(9, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.65, rate.get_rate_to_base());
    FT_ASSERT_EQ(6, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_getters_reset_errno, "Currency rate getters set errno to success after access")
{
    ft_currency_rate rate(3, 1.1, 4);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(3, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.1, rate.get_rate_to_base());
    FT_ASSERT_EQ(4, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_self_assignments_retain_state, "Self copy and move assignments leave rate unchanged")
{
    ft_currency_rate rate(28, 3.5, 1);

    rate = rate;
    rate = ft_move(rate);

    FT_ASSERT_EQ(28, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.5, rate.get_rate_to_base());
    FT_ASSERT_EQ(1, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}
