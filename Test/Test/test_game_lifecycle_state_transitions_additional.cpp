#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_lifecycle_initialize_destroy_success, "price definition initialize then destroy succeeds")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_destroy_uninitialized_invalid_state, "price definition destroy on uninitialized returns invalid state")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_reinitialize_after_destroy_success, "price definition can initialize again after destroy")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_lifecycle_disable_thread_safety_uninitialized_success, "price definition disable_thread_safety on uninitialized succeeds")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_initialize_destroy_success, "rarity band initialize then destroy succeeds")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_destroy_uninitialized_invalid_state, "rarity band destroy on uninitialized returns invalid state")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_reinitialize_after_destroy_success, "rarity band can initialize again after destroy")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_lifecycle_disable_thread_safety_uninitialized_success, "rarity band disable_thread_safety on uninitialized succeeds")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_initialize_destroy_success, "vendor profile initialize then destroy succeeds")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_destroy_uninitialized_invalid_state, "vendor profile destroy on uninitialized returns invalid state")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_reinitialize_after_destroy_success, "vendor profile can initialize again after destroy")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_lifecycle_disable_thread_safety_uninitialized_success, "vendor profile disable_thread_safety on uninitialized succeeds")
{
    ft_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_initialize_destroy_success, "currency rate initialize then destroy succeeds")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_destroy_uninitialized_invalid_state, "currency rate destroy on uninitialized returns invalid state")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_reinitialize_after_destroy_success, "currency rate can initialize again after destroy")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_lifecycle_disable_thread_safety_uninitialized_success, "currency rate disable_thread_safety on uninitialized succeeds")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_initialize_destroy_success, "economy table initialize then destroy succeeds")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_destroy_uninitialized_invalid_state, "economy table destroy on uninitialized returns invalid state")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_reinitialize_after_destroy_success, "economy table can initialize again after destroy")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_lifecycle_disable_thread_safety_uninitialized_success, "economy table disable_thread_safety on uninitialized succeeds")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
