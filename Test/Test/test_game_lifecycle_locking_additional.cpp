#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_lock_without_thread_safety_reports_not_acquired, "price definition lock succeeds without mutex and reports not acquired")
{
    ft_price_definition definition;
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_price_lock_with_thread_safety_reports_acquired, "price definition lock acquires mutex when thread safety is enabled")
{
    ft_price_definition definition;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    definition.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_price_lock_unlock_round_trip_with_mutex, "price definition lock and unlock round-trip succeeds with mutex")
{
    ft_price_definition definition;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    definition.unlock(lock_acquired);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    definition.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_price_recursive_lock_unlock_with_mutex, "price definition supports recursive lock and balanced unlock")
{
    ft_price_definition definition;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first_lock_acquired = false;
    second_lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&first_lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&second_lock_acquired));
    FT_ASSERT_EQ(true, first_lock_acquired);
    FT_ASSERT_EQ(true, second_lock_acquired);
    definition.unlock(second_lock_acquired);
    definition.unlock(first_lock_acquired);
    return (1);
}

FT_TEST(test_rarity_lock_without_thread_safety_reports_not_acquired, "rarity band lock succeeds without mutex and reports not acquired")
{
    ft_rarity_band band;
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_rarity_lock_with_thread_safety_reports_acquired, "rarity band lock acquires mutex when thread safety is enabled")
{
    ft_rarity_band band;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    band.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_rarity_lock_unlock_round_trip_with_mutex, "rarity band lock and unlock round-trip succeeds with mutex")
{
    ft_rarity_band band;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    band.unlock(lock_acquired);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    band.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_rarity_recursive_lock_unlock_with_mutex, "rarity band supports recursive lock and balanced unlock")
{
    ft_rarity_band band;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first_lock_acquired = false;
    second_lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&first_lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&second_lock_acquired));
    FT_ASSERT_EQ(true, first_lock_acquired);
    FT_ASSERT_EQ(true, second_lock_acquired);
    band.unlock(second_lock_acquired);
    band.unlock(first_lock_acquired);
    return (1);
}

FT_TEST(test_vendor_lock_without_thread_safety_reports_not_acquired, "vendor profile lock succeeds without mutex and reports not acquired")
{
    ft_vendor_profile profile;
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_vendor_lock_with_thread_safety_reports_acquired, "vendor profile lock acquires mutex when thread safety is enabled")
{
    ft_vendor_profile profile;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    profile.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_vendor_lock_unlock_round_trip_with_mutex, "vendor profile lock and unlock round-trip succeeds with mutex")
{
    ft_vendor_profile profile;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    profile.unlock(lock_acquired);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    profile.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_vendor_recursive_lock_unlock_with_mutex, "vendor profile supports recursive lock and balanced unlock")
{
    ft_vendor_profile profile;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first_lock_acquired = false;
    second_lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&first_lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&second_lock_acquired));
    FT_ASSERT_EQ(true, first_lock_acquired);
    FT_ASSERT_EQ(true, second_lock_acquired);
    profile.unlock(second_lock_acquired);
    profile.unlock(first_lock_acquired);
    return (1);
}

FT_TEST(test_currency_lock_without_thread_safety_reports_not_acquired, "currency rate lock succeeds without mutex and reports not acquired")
{
    ft_currency_rate rate;
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_currency_lock_with_thread_safety_reports_acquired, "currency rate lock acquires mutex when thread safety is enabled")
{
    ft_currency_rate rate;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    rate.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_currency_lock_unlock_round_trip_with_mutex, "currency rate lock and unlock round-trip succeeds with mutex")
{
    ft_currency_rate rate;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    rate.unlock(lock_acquired);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    rate.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_currency_recursive_lock_unlock_with_mutex, "currency rate supports recursive lock and balanced unlock")
{
    ft_currency_rate rate;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first_lock_acquired = false;
    second_lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&first_lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&second_lock_acquired));
    FT_ASSERT_EQ(true, first_lock_acquired);
    FT_ASSERT_EQ(true, second_lock_acquired);
    rate.unlock(second_lock_acquired);
    rate.unlock(first_lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_lock_without_thread_safety_reports_not_acquired, "economy table lock succeeds without mutex and reports not acquired")
{
    ft_economy_table table;
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_lock_with_thread_safety_reports_acquired, "economy table lock acquires mutex when thread safety is enabled")
{
    ft_economy_table table;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    table.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_lock_unlock_round_trip_with_mutex, "economy table lock and unlock round-trip succeeds with mutex")
{
    ft_economy_table table;
    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    table.unlock(lock_acquired);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    table.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_recursive_lock_unlock_with_mutex, "economy table supports recursive lock and balanced unlock")
{
    ft_economy_table table;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first_lock_acquired = false;
    second_lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&first_lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&second_lock_acquired));
    FT_ASSERT_EQ(true, first_lock_acquired);
    FT_ASSERT_EQ(true, second_lock_acquired);
    table.unlock(second_lock_acquired);
    table.unlock(first_lock_acquired);
    return (1);
}
