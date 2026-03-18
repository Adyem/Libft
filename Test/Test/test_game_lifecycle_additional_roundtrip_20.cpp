#include "../test_internal.hpp"
#include "../../Game/game_price_definition.hpp"
#include "../../Game/game_rarity_band.hpp"
#include "../../Game/game_vendor_profile.hpp"
#include "../../Game/game_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_roundtrip_destroy_twice_reports_invalid_state)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.destroy());
    return (1);
}

FT_TEST(test_price_roundtrip_enable_then_destroy_succeeds)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    return (1);
}

FT_TEST(test_price_roundtrip_destroy_then_reinitialize_with_values)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(50, 5, 800, 400, 1200));
    FT_ASSERT_EQ(50, definition.get_item_id());
    return (1);
}

FT_TEST(test_price_roundtrip_disable_thread_safety_after_destroy_succeeds)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.disable_thread_safety());
    return (1);
}

FT_TEST(test_rarity_roundtrip_destroy_twice_reports_invalid_state)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.destroy());
    return (1);
}

FT_TEST(test_rarity_roundtrip_enable_then_destroy_succeeds)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    return (1);
}

FT_TEST(test_rarity_roundtrip_destroy_then_reinitialize_with_values)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(9, 1.7));
    FT_ASSERT_EQ(9, band.get_rarity());
    return (1);
}

FT_TEST(test_rarity_roundtrip_disable_thread_safety_after_destroy_succeeds)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.disable_thread_safety());
    return (1);
}

FT_TEST(test_vendor_roundtrip_destroy_twice_reports_invalid_state)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.destroy());
    return (1);
}

FT_TEST(test_vendor_roundtrip_enable_then_destroy_succeeds)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    return (1);
}

FT_TEST(test_vendor_roundtrip_destroy_then_reinitialize_with_values)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(14, 1.25, 0.65, 0.07));
    FT_ASSERT_EQ(14, profile.get_vendor_id());
    return (1);
}

FT_TEST(test_vendor_roundtrip_disable_thread_safety_after_destroy_succeeds)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.disable_thread_safety());
    return (1);
}

FT_TEST(test_currency_roundtrip_destroy_twice_reports_invalid_state)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.destroy());
    return (1);
}

FT_TEST(test_currency_roundtrip_enable_then_destroy_succeeds)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    return (1);
}

FT_TEST(test_currency_roundtrip_destroy_then_reinitialize_with_values)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(21, 3.5, 4));
    FT_ASSERT_EQ(21, rate.get_currency_id());
    return (1);
}

FT_TEST(test_currency_roundtrip_disable_thread_safety_after_destroy_succeeds)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.disable_thread_safety());
    return (1);
}

FT_TEST(test_table_roundtrip_destroy_twice_reports_invalid_state)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    return (1);
}

FT_TEST(test_table_roundtrip_enable_then_destroy_succeeds)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    return (1);
}

FT_TEST(test_table_roundtrip_destroy_then_reinitialize_succeeds)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    return (1);
}

FT_TEST(test_table_roundtrip_disable_thread_safety_after_destroy_succeeds)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    return (1);
}
