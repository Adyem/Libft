#include "../test_internal.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/map.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_lifecycle_signal_caught = 0;
static sigjmp_buf g_lifecycle_signal_jump_buffer;

static void lifecycle_signal_handler(int signal_value)
{
    g_lifecycle_signal_caught = signal_value;
    siglongjmp(g_lifecycle_signal_jump_buffer, 1);
    return ;
}

static int expect_sigabrt_on_uninitialised_table(void (*operation)(game_economy_table &))
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &lifecycle_signal_handler;
    new_action_iot.sa_handler = &lifecycle_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (SIGIOT != SIGABRT && sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    g_lifecycle_signal_caught = 0;
    jump_result = sigsetjmp(g_lifecycle_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        alignas(game_economy_table) unsigned char storage[sizeof(game_economy_table)];
        game_economy_table *table_pointer;

        std::memset(storage, 0, sizeof(storage));
        table_pointer = reinterpret_cast<game_economy_table *>(storage);
        operation(*table_pointer);
    }
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    if (SIGIOT != SIGABRT)
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_lifecycle_signal_caught == SIGABRT)
        return (1);
    return (g_lifecycle_signal_caught == SIGIOT);
}

static void operation_register_price(game_economy_table &table)
{
    game_price_definition definition;

    (void)definition.initialize(10, 2, 120, 80, 200);
    (void)table.register_price_definition(definition);
    return ;
}

static void operation_register_rarity(game_economy_table &table)
{
    game_rarity_band band;

    (void)band.initialize(3, 1.2);
    (void)table.register_rarity_band(band);
    return ;
}

static void operation_register_vendor(game_economy_table &table)
{
    game_vendor_profile profile;

    (void)profile.initialize(5, 1.1, 0.7, 0.05);
    (void)table.register_vendor_profile(profile);
    return ;
}

static void operation_register_currency(game_economy_table &table)
{
    game_currency_rate rate;

    (void)rate.initialize(7, 2.0, 2);
    (void)table.register_currency_rate(rate);
    return ;
}

static void operation_fetch_price(game_economy_table &table)
{
    game_price_definition definition;

    (void)definition.initialize();
    (void)table.fetch_price_definition(9, definition);
    return ;
}

static void operation_fetch_rarity(game_economy_table &table)
{
    game_rarity_band band;

    (void)band.initialize();
    (void)table.fetch_rarity_band(4, band);
    return ;
}

static void operation_fetch_vendor(game_economy_table &table)
{
    game_vendor_profile profile;

    (void)table.fetch_vendor_profile(8, profile);
    return ;
}

static void operation_fetch_currency(game_economy_table &table)
{
    game_currency_rate rate;

    (void)rate.initialize();
    (void)table.fetch_currency_rate(6, rate);
    return ;
}

static void operation_get_price_definitions(game_economy_table &table)
{
    (void)table.get_price_definitions();
    return ;
}

static void operation_get_rarity_bands(game_economy_table &table)
{
    (void)table.get_rarity_bands();
    return ;
}

static void operation_get_vendor_profiles(game_economy_table &table)
{
    (void)table.get_vendor_profiles();
    return ;
}

static void operation_get_currency_rates(game_economy_table &table)
{
    (void)table.get_currency_rates();
    return ;
}

static void operation_set_price_definitions(game_economy_table &table)
{
    ft_map<int, game_price_definition> definitions;

    table.set_price_definitions(definitions);
    return ;
}

static void operation_set_rarity_bands(game_economy_table &table)
{
    ft_map<int, game_rarity_band> bands;

    table.set_rarity_bands(bands);
    return ;
}

static void operation_set_vendor_profiles(game_economy_table &table)
{
    ft_map<int, game_vendor_profile> profiles;

    table.set_vendor_profiles(profiles);
    return ;
}

static void operation_set_currency_rates(game_economy_table &table)
{
    ft_map<int, game_currency_rate> rates;

    table.set_currency_rates(rates);
    return ;
}

static void operation_enable_thread_safety(game_economy_table &table)
{
    (void)table.enable_thread_safety();
    return ;
}

static void operation_is_thread_safe(game_economy_table &table)
{
    (void)table.is_thread_safe();
    return ;
}

static void operation_lock(game_economy_table &table)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    (void)table.lock(&lock_acquired);
    return ;
}

static void operation_unlock(game_economy_table &table)
{
    table.unlock(FT_FALSE);
    return ;
}

FT_TEST(test_economy_lifecycle_register_price_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_register_price));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_register_rarity));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_vendor_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_register_vendor));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_currency_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_register_currency));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_price_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_fetch_price));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_fetch_rarity));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_vendor_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_fetch_vendor));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_currency_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_fetch_currency));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_price_definitions_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_get_price_definitions));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_rarity_bands_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_get_rarity_bands));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_vendor_profiles_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_get_vendor_profiles));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_currency_rates_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_get_currency_rates));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_price_definitions_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_set_price_definitions));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_rarity_bands_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_set_rarity_bands));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_vendor_profiles_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_set_vendor_profiles));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_currency_rates_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_set_currency_rates));
    return (1);
}

FT_TEST(test_economy_lifecycle_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_enable_thread_safety));
    return (1);
}

FT_TEST(test_economy_lifecycle_is_thread_safe_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_table(operation_is_thread_safe));
    return (1);
}

FT_TEST(test_economy_lifecycle_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_lock));
    return (1);
}

FT_TEST(test_economy_lifecycle_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_table(operation_unlock));
    return (1);
}
