#include "../test_internal.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_price_lock_without_thread_safety_reports_not_acquired)
{
    game_price_definition definition;
    ft_bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_price_lock_with_thread_safety_reports_acquired)
{
    game_price_definition definition;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    definition.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_price_lock_unlock_round_trip_with_mutex)
{
    game_price_definition definition;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    definition.unlock(lock_acquired);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    definition.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_price_recursive_lock_unlock_with_mutex)
{
    game_price_definition definition;
    ft_bool first_lock_acquired;
    ft_bool second_lock_acquired;

    first_lock_acquired = FT_FALSE;
    second_lock_acquired = FT_FALSE;
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

FT_TEST(test_rarity_lock_without_thread_safety_reports_not_acquired)
{
    game_rarity_band band;
    ft_bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_rarity_lock_with_thread_safety_reports_acquired)
{
    game_rarity_band band;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    band.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_rarity_lock_unlock_round_trip_with_mutex)
{
    game_rarity_band band;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    band.unlock(lock_acquired);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    band.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_rarity_recursive_lock_unlock_with_mutex)
{
    game_rarity_band band;
    ft_bool first_lock_acquired;
    ft_bool second_lock_acquired;

    first_lock_acquired = FT_FALSE;
    second_lock_acquired = FT_FALSE;
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

FT_TEST(test_vendor_lock_without_thread_safety_reports_not_acquired)
{
    game_vendor_profile profile;
    ft_bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_vendor_lock_with_thread_safety_reports_acquired)
{
    game_vendor_profile profile;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    profile.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_vendor_lock_unlock_round_trip_with_mutex)
{
    game_vendor_profile profile;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    profile.unlock(lock_acquired);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    profile.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_vendor_recursive_lock_unlock_with_mutex)
{
    game_vendor_profile profile;
    ft_bool first_lock_acquired;
    ft_bool second_lock_acquired;

    first_lock_acquired = FT_FALSE;
    second_lock_acquired = FT_FALSE;
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

FT_TEST(test_currency_lock_without_thread_safety_reports_not_acquired)
{
    game_currency_rate rate;
    ft_bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_currency_lock_with_thread_safety_reports_acquired)
{
    game_currency_rate rate;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    rate.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_currency_lock_unlock_round_trip_with_mutex)
{
    game_currency_rate rate;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    rate.unlock(lock_acquired);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    rate.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_currency_recursive_lock_unlock_with_mutex)
{
    game_currency_rate rate;
    ft_bool first_lock_acquired;
    ft_bool second_lock_acquired;

    first_lock_acquired = FT_FALSE;
    second_lock_acquired = FT_FALSE;
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

FT_TEST(test_economy_table_lock_without_thread_safety_reports_not_acquired)
{
    game_economy_table table;
    ft_bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_lock_with_thread_safety_reports_acquired)
{
    game_economy_table table;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    table.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_lock_unlock_round_trip_with_mutex)
{
    game_economy_table table;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    table.unlock(lock_acquired);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    table.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_economy_table_recursive_lock_unlock_with_mutex)
{
    game_economy_table table;
    ft_bool first_lock_acquired;
    ft_bool second_lock_acquired;

    first_lock_acquired = FT_FALSE;
    second_lock_acquired = FT_FALSE;
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
