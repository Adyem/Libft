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

FT_TEST(test_price_thread_safety_enable_sets_state)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(true, definition.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_thread_safety_disable_clears_state)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(false, definition.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_thread_safety_enable_twice_succeeds)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(true, definition.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_price_thread_safety_disable_without_enable_succeeds)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    FT_ASSERT_EQ(false, definition.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_rarity_thread_safety_enable_sets_state)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(true, band.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_thread_safety_disable_clears_state)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(false, band.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_thread_safety_enable_twice_succeeds)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(true, band.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_rarity_thread_safety_disable_without_enable_succeeds)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    FT_ASSERT_EQ(false, band.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
    return (1);
}

FT_TEST(test_vendor_thread_safety_enable_sets_state)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(true, profile.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_thread_safety_disable_clears_state)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(false, profile.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_thread_safety_enable_twice_succeeds)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(true, profile.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_vendor_thread_safety_disable_without_enable_succeeds)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(false, profile.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_currency_thread_safety_enable_sets_state)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(true, rate.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_thread_safety_disable_clears_state)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(false, rate.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_thread_safety_enable_twice_succeeds)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(true, rate.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_currency_thread_safety_disable_without_enable_succeeds)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    FT_ASSERT_EQ(false, rate.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
    return (1);
}

FT_TEST(test_economy_table_thread_safety_enable_sets_state)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(true, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_thread_safety_disable_clears_state)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(false, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_thread_safety_enable_twice_succeeds)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(true, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_economy_table_thread_safety_disable_without_enable_succeeds)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(false, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
