#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_currency_rate_default_constructor_values, "Currency rate default constructor initializes baseline values")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(0, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.0, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_parameterized_constructor_values, "Currency rate parameterized constructor stores provided values")
{
    ft_currency_rate rate(42, 3.75, 5);

    FT_ASSERT_EQ(42, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_copy_constructor_clones_values, "Currency rate copy constructor clones stored values")
{
    ft_currency_rate original(8, 0.25, 4);
    ft_currency_rate copy(original);

    FT_ASSERT_EQ(8, copy.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.25, copy.get_rate_to_base());
    FT_ASSERT_EQ(4, copy.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.get_error());
    return (1);
}


FT_TEST(test_currency_rate_copy_assignment_overwrites_destination, "Currency rate copy assignment overwrites destination values")
{
    ft_currency_rate source(13, 1.2, 6);
    ft_currency_rate destination(5, 0.5, 1);

    destination = source;

    FT_ASSERT_EQ(13, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.2, destination.get_rate_to_base());
    FT_ASSERT_EQ(6, destination.get_display_precision());
    FT_ASSERT_EQ(5, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, source.get_rate_to_base());
    FT_ASSERT_EQ(1, source.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_move_constructor_transfers_values, "Currency rate move constructor transfers values and clears source")
{
    ft_currency_rate original(21, 2.5, 7);
    ft_currency_rate moved(ft_move(original));

    FT_ASSERT_EQ(21, moved.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(2.5, moved.get_rate_to_base());
    FT_ASSERT_EQ(7, moved.get_display_precision());
    FT_ASSERT_EQ(0, original.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, original.get_rate_to_base());
    FT_ASSERT_EQ(0, original.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_move_assignment_transfers_values, "Currency rate move assignment transfers values and resets source")
{
    ft_currency_rate source(19, 4.2, 3);
    ft_currency_rate destination(2, 0.8, 9);

    destination = ft_move(source);

    FT_ASSERT_EQ(19, destination.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(4.2, destination.get_rate_to_base());
    FT_ASSERT_EQ(3, destination.get_display_precision());
    FT_ASSERT_EQ(0, source.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_rate_to_base());
    FT_ASSERT_EQ(0, source.get_display_precision());
    return (1);
}


FT_TEST(test_currency_rate_setters_update_values, "Currency rate setters update stored fields")
{
    ft_currency_rate rate;

    rate.set_currency_id(77);
    rate.set_rate_to_base(1.75);
    rate.set_display_precision(8);

    FT_ASSERT_EQ(77, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(1.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(8, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_self_copy_assignment_preserves_state, "Currency rate self copy assignment preserves state")
{
    ft_currency_rate rate(9, 3.3, 4);

    rate = rate;

    FT_ASSERT_EQ(9, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.3, rate.get_rate_to_base());
    FT_ASSERT_EQ(4, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_currency_rate_self_move_assignment_preserves_state, "Currency rate self move assignment preserves state")
{
    ft_currency_rate rate(17, 0.95, 2);

    rate = ft_move(rate);

    FT_ASSERT_EQ(17, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.95, rate.get_rate_to_base());
    FT_ASSERT_EQ(2, rate.get_display_precision());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, rate.get_error());
    return (1);
}


FT_TEST(test_price_definition_setters_apply_updates, "Price definition setters update stored values")
{
    ft_price_definition definition;

    definition.set_item_id(11);
    definition.set_rarity(5);
    definition.set_base_value(220);
    definition.set_minimum_value(120);
    definition.set_maximum_value(400);

    FT_ASSERT_EQ(11, definition.get_item_id());
    FT_ASSERT_EQ(5, definition.get_rarity());
    FT_ASSERT_EQ(220, definition.get_base_value());
    FT_ASSERT_EQ(120, definition.get_minimum_value());
    FT_ASSERT_EQ(400, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, definition.get_error());
    return (1);
}
