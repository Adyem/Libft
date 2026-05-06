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

#ifndef LIBFT_TEST_BUILD
#endif

static int lifecycle_expect_sigabrt(void (*operation)(void))
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

static void price_call_disable_thread_safety_uninitialised(void)
{
    game_price_definition definition;

    (void)definition.disable_thread_safety();
    return ;
}

static void rarity_call_disable_thread_safety_uninitialised(void)
{
    game_rarity_band band;

    (void)band.disable_thread_safety();
    return ;
}

static void vendor_call_disable_thread_safety_uninitialised(void)
{
    game_vendor_profile profile;

    (void)profile.disable_thread_safety();
    return ;
}

static void currency_call_disable_thread_safety_uninitialised(void)
{
    game_currency_rate rate;

    (void)rate.disable_thread_safety();
    return ;
}

static void economy_table_call_disable_thread_safety_uninitialised(void)
{
    game_economy_table table;

    (void)table.disable_thread_safety();
    return ;
}

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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
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

FT_TEST(test_price_lifecycle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(
        price_call_disable_thread_safety_uninitialised));
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.get_error());
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

FT_TEST(test_rarity_lifecycle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(
        rarity_call_disable_thread_safety_uninitialised));
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
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

FT_TEST(test_vendor_lifecycle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(
        vendor_call_disable_thread_safety_uninitialised));
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.get_error());
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

FT_TEST(test_currency_lifecycle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(
        currency_call_disable_thread_safety_uninitialised));
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
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

FT_TEST(test_economy_table_lifecycle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(
        economy_table_call_disable_thread_safety_uninitialised));
    return (1);
}
