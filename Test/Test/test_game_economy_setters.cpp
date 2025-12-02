#include "../../Game/game_achievement.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_price_definition_setters_update_values, "Game: price definition setters apply new values")
{
    ft_price_definition definition;

    definition.set_item_id(12);
    definition.set_rarity(3);
    definition.set_base_value(150);
    definition.set_minimum_value(100);
    definition.set_maximum_value(250);
    FT_ASSERT_EQ(12, definition.get_item_id());
    FT_ASSERT_EQ(3, definition.get_rarity());
    FT_ASSERT_EQ(150, definition.get_base_value());
    FT_ASSERT_EQ(100, definition.get_minimum_value());
    FT_ASSERT_EQ(250, definition.get_maximum_value());
    FT_ASSERT_EQ(ER_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_refresh_multiplier, "Game: rarity band setters update rarity and multiplier")
{
    ft_rarity_band band;

    band.set_rarity(6);
    band.set_value_multiplier(2.0);
    FT_ASSERT_EQ(6, band.get_rarity());
    FT_ASSERT_DOUBLE_EQ(2.0, band.get_value_multiplier());
    band.set_value_multiplier(1.25);
    FT_ASSERT_DOUBLE_EQ(1.25, band.get_value_multiplier());
    FT_ASSERT_EQ(ER_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_currency_rate_setters_apply_changes, "Game: currency rate setters replace stored values")
{
    ft_currency_rate rate;

    rate.set_currency_id(8);
    rate.set_rate_to_base(3.75);
    rate.set_display_precision(5);
    FT_ASSERT_EQ(8, rate.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(3.75, rate.get_rate_to_base());
    FT_ASSERT_EQ(5, rate.get_display_precision());
    FT_ASSERT_EQ(ER_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_goal_progress_accumulates_through_setters, "Game: goal progress setters accumulate progress")
{
    ft_goal goal;

    goal.set_target(4);
    goal.set_progress(1);
    goal.add_progress(2);
    FT_ASSERT_EQ(4, goal.get_target());
    FT_ASSERT_EQ(3, goal.get_progress());
    goal.add_progress(1);
    FT_ASSERT_EQ(4, goal.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, goal.get_error());
    return (1);
}

FT_TEST(test_reputation_current_and_total_tracking, "Game: reputation tracks current and total reps independently")
{
    ft_reputation reputation;

    reputation.set_total_rep(10);
    reputation.set_current_rep(4);
    reputation.add_total_rep(6);
    reputation.add_current_rep(3);
    FT_ASSERT_EQ(19, reputation.get_total_rep());
    FT_ASSERT_EQ(7, reputation.get_current_rep());
    reputation.sub_total_rep(1);
    reputation.sub_current_rep(2);
    FT_ASSERT_EQ(16, reputation.get_total_rep());
    FT_ASSERT_EQ(5, reputation.get_current_rep());
    FT_ASSERT_EQ(ER_SUCCESS, reputation.get_error());
    return (1);
}
