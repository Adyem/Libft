#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_resistance.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_value_currency_zero_rate)
{
    game_currency_rate value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 0.0, 0));
    FT_ASSERT_DOUBLE_EQ(0.0, value.get_rate_to_base());
    return (1);
}

FT_TEST(test_game_value_currency_negative_rate)
{
    game_currency_rate value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(2, -1.25, 4));
    FT_ASSERT_EQ(2, value.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(-1.25, value.get_rate_to_base());
    return (1);
}

FT_TEST(test_game_value_currency_precision_round_trip)
{
    game_currency_rate value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_display_precision(0);
    FT_ASSERT_EQ(0, value.get_display_precision());
    value.set_display_precision(12);
    FT_ASSERT_EQ(12, value.get_display_precision());
    return (1);
}

FT_TEST(test_game_value_currency_copy_is_independent)
{
    game_currency_rate source;
    game_currency_rate copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 2.5, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    source.set_rate_to_base(9.0);
    FT_ASSERT_DOUBLE_EQ(2.5, copy.get_rate_to_base());
    return (1);
}

FT_TEST(test_game_value_currency_thread_safety_retry)
{
    game_currency_rate value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_value_resistance_negative_values)
{
    game_resistance value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(-20, -7));
    FT_ASSERT_EQ(-20, value.get_percent());
    FT_ASSERT_EQ(-7, value.get_flat());
    return (1);
}

FT_TEST(test_game_value_resistance_zero_reset)
{
    game_resistance value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(80, 15));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.reset());
    FT_ASSERT_EQ(0, value.get_percent());
    FT_ASSERT_EQ(0, value.get_flat());
    return (1);
}

FT_TEST(test_game_value_resistance_copy_preserves_both_fields)
{
    game_resistance source;
    game_resistance copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_values(33, 11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(33, copy.get_percent());
    FT_ASSERT_EQ(11, copy.get_flat());
    return (1);
}

FT_TEST(test_game_value_resistance_move_clears_source)
{
    game_resistance source;
    game_resistance destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_values(44, 22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_resistance &&>(source)));
    FT_ASSERT_EQ(44, destination.get_percent());
    FT_ASSERT_EQ(22, destination.get_flat());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_value_resistance_thread_safety_idempotent)
{
    game_resistance value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_value_price_parameterized_edges)
{
    game_price_definition value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(9, 0, 0, -10, 10));
    FT_ASSERT_EQ(9, value.get_item_id());
    FT_ASSERT_EQ(-10, value.get_minimum_value());
    FT_ASSERT_EQ(10, value.get_maximum_value());
    return (1);
}

FT_TEST(test_game_value_price_copy_after_mutation)
{
    game_price_definition source;
    game_price_definition copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(1, 2, 30, 20, 40));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    source.set_base_value(99);
    FT_ASSERT_EQ(30, copy.get_base_value());
    return (1);
}

FT_TEST(test_game_value_price_move_transfers_all_fields)
{
    game_price_definition source;
    game_price_definition destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4, 5, 60, 50, 70));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_price_definition &&>(source)));
    FT_ASSERT_EQ(4, destination.get_item_id());
    FT_ASSERT_EQ(5, destination.get_rarity());
    FT_ASSERT_EQ(70, destination.get_maximum_value());
    return (1);
}

FT_TEST(test_game_value_price_thread_safety_round_trip)
{
    game_price_definition value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_value_price_error_string_success)
{
    game_price_definition value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_STR_EQ("Success", value.get_error_str());
    return (1);
}

FT_TEST(test_game_value_rarity_parameterized_zero)
{
    game_rarity_band value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(0, 0.0));
    FT_ASSERT_EQ(0, value.get_rarity());
    FT_ASSERT_DOUBLE_EQ(0.0, value.get_value_multiplier());
    return (1);
}

FT_TEST(test_game_value_rarity_negative_multiplier)
{
    game_rarity_band value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(-1, -2.0));
    FT_ASSERT_DOUBLE_EQ(-2.0, value.get_value_multiplier());
    return (1);
}

FT_TEST(test_game_value_rarity_copy_independence)
{
    game_rarity_band source;
    game_rarity_band copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(6, 1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    source.set_rarity(8);
    FT_ASSERT_EQ(6, copy.get_rarity());
    return (1);
}

FT_TEST(test_game_value_rarity_move_source_destroyed)
{
    game_rarity_band source;
    game_rarity_band destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, 3.25));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_rarity_band &&>(source)));
    FT_ASSERT_EQ(7, destination.get_rarity());
    FT_ASSERT_DOUBLE_EQ(3.25, destination.get_value_multiplier());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_value_rarity_thread_safety_disable_without_enable)
{
    game_rarity_band value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}
