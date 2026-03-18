#include "../test_internal.hpp"
#include "../../Game/game_price_definition.hpp"
#include "../../Game/game_rarity_band.hpp"
#include "../../Game/game_vendor_profile.hpp"
#include "../../Game/game_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_lifecycle_initialize_destroy_success)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_destroy_uninitialised_invalid_state)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_reinitialize_after_destroy_success)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_disable_thread_safety_uninitialised_success)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_destroy_success)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_destroy_uninitialised_invalid_state)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_reinitialize_after_destroy_success)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_disable_thread_safety_uninitialised_success)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_destroy_success)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_destroy_uninitialised_invalid_state)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_reinitialize_after_destroy_success)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_disable_thread_safety_uninitialised_success)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_destroy_success)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_destroy_uninitialised_invalid_state)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_reinitialize_after_destroy_success)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_disable_thread_safety_uninitialised_success)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_destroy_success)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_destroy_uninitialised_invalid_state)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_reinitialize_after_destroy_success)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_disable_thread_safety_uninitialised_success)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
