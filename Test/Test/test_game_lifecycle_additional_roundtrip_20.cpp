#include "../test_internal.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int lifecycle_roundtrip_expect_sigabrt(void (*operation)(void))
{
    struct sigaction new_action;
    struct sigaction old_action;
    static volatile sig_atomic_t abort_captured = 0;
    static sigjmp_buf abort_jump_buffer;
    void (*previous_handler)(int);

    previous_handler = ft_nullptr;
    std::memset(&new_action, 0, sizeof(new_action));
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    new_action.sa_handler = +[](int signal_number)
    {
        (void)signal_number;
        abort_captured = 1;
        siglongjmp(abort_jump_buffer, 1);
    };
    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
        return (0);
    abort_captured = 0;
    if (sigsetjmp(abort_jump_buffer, 1) == 0)
        operation();
    previous_handler = old_action.sa_handler;
    if (sigaction(SIGABRT, &old_action, ft_nullptr) != 0)
        return (0);
    (void)previous_handler;
    return (abort_captured != 0);
}

static void price_call_disable_thread_safety_after_destroy(void)
{
    game_price_definition definition;

    (void)definition.initialize();
    (void)definition.destroy();
    (void)definition.disable_thread_safety();
    return ;
}

static void rarity_call_disable_thread_safety_after_destroy(void)
{
    game_rarity_band band;

    (void)band.initialize();
    (void)band.destroy();
    (void)band.disable_thread_safety();
    return ;
}

static void vendor_call_disable_thread_safety_after_destroy(void)
{
    game_vendor_profile profile;

    (void)profile.initialize();
    (void)profile.destroy();
    (void)profile.disable_thread_safety();
    return ;
}

static void currency_call_disable_thread_safety_after_destroy(void)
{
    game_currency_rate rate;

    (void)rate.initialize();
    (void)rate.destroy();
    (void)rate.disable_thread_safety();
    return ;
}

static void table_call_disable_thread_safety_after_destroy(void)
{
    game_economy_table table;

    (void)table.initialize();
    (void)table.destroy();
    (void)table.disable_thread_safety();
    return ;
}

FT_TEST(test_price_roundtrip_destroy_twice_reports_invalid_state)
{
    game_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
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

FT_TEST(test_price_roundtrip_disable_thread_safety_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_roundtrip_expect_sigabrt(
        price_call_disable_thread_safety_after_destroy));
    return (1);
}

FT_TEST(test_rarity_roundtrip_destroy_twice_reports_invalid_state)
{
    game_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
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

FT_TEST(test_rarity_roundtrip_disable_thread_safety_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_roundtrip_expect_sigabrt(
        rarity_call_disable_thread_safety_after_destroy));
    return (1);
}

FT_TEST(test_vendor_roundtrip_destroy_twice_reports_invalid_state)
{
    game_vendor_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
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

FT_TEST(test_vendor_roundtrip_disable_thread_safety_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_roundtrip_expect_sigabrt(
        vendor_call_disable_thread_safety_after_destroy));
    return (1);
}

FT_TEST(test_currency_roundtrip_destroy_twice_reports_invalid_state)
{
    game_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
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

FT_TEST(test_currency_roundtrip_disable_thread_safety_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_roundtrip_expect_sigabrt(
        currency_call_disable_thread_safety_after_destroy));
    return (1);
}

FT_TEST(test_table_roundtrip_destroy_twice_reports_invalid_state)
{
    game_economy_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
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

FT_TEST(test_table_roundtrip_disable_thread_safety_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_roundtrip_expect_sigabrt(
        table_call_disable_thread_safety_after_destroy));
    return (1);
}
