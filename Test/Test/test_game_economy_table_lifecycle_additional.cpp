#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

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

static int expect_sigabrt_on_uninitialized_table(void (*operation)(ft_economy_table &))
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
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    g_lifecycle_signal_caught = 0;
    jump_result = sigsetjmp(g_lifecycle_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        alignas(ft_economy_table) unsigned char storage[sizeof(ft_economy_table)];
        ft_economy_table *table_pointer;

        std::memset(storage, 0, sizeof(storage));
        table_pointer = reinterpret_cast<ft_economy_table *>(storage);
        operation(*table_pointer);
    }
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_lifecycle_signal_caught == SIGABRT)
        return (1);
    return (g_lifecycle_signal_caught == SIGIOT);
}

static void operation_register_price(ft_economy_table &table)
{
    ft_price_definition definition;

    (void)definition.initialize(10, 2, 120, 80, 200);
    (void)table.register_price_definition(definition);
    return ;
}

static void operation_register_rarity(ft_economy_table &table)
{
    ft_rarity_band band;

    (void)band.initialize(3, 1.2);
    (void)table.register_rarity_band(band);
    return ;
}

static void operation_register_vendor(ft_economy_table &table)
{
    ft_vendor_profile profile;

    (void)profile.initialize(5, 1.1, 0.7, 0.05);
    (void)table.register_vendor_profile(profile);
    return ;
}

static void operation_register_currency(ft_economy_table &table)
{
    ft_currency_rate rate;

    (void)rate.initialize(7, 2.0, 2);
    (void)table.register_currency_rate(rate);
    return ;
}

static void operation_fetch_price(ft_economy_table &table)
{
    ft_price_definition definition;

    (void)definition.initialize();
    (void)table.fetch_price_definition(9, definition);
    return ;
}

static void operation_fetch_rarity(ft_economy_table &table)
{
    ft_rarity_band band;

    (void)band.initialize();
    (void)table.fetch_rarity_band(4, band);
    return ;
}

static void operation_fetch_vendor(ft_economy_table &table)
{
    ft_vendor_profile profile;

    (void)table.fetch_vendor_profile(8, profile);
    return ;
}

static void operation_fetch_currency(ft_economy_table &table)
{
    ft_currency_rate rate;

    (void)rate.initialize();
    (void)table.fetch_currency_rate(6, rate);
    return ;
}

static void operation_get_price_definitions(ft_economy_table &table)
{
    (void)table.get_price_definitions();
    return ;
}

static void operation_get_rarity_bands(ft_economy_table &table)
{
    (void)table.get_rarity_bands();
    return ;
}

static void operation_get_vendor_profiles(ft_economy_table &table)
{
    (void)table.get_vendor_profiles();
    return ;
}

static void operation_get_currency_rates(ft_economy_table &table)
{
    (void)table.get_currency_rates();
    return ;
}

static void operation_set_price_definitions(ft_economy_table &table)
{
    ft_map<int, ft_price_definition> definitions;

    table.set_price_definitions(definitions);
    return ;
}

static void operation_set_rarity_bands(ft_economy_table &table)
{
    ft_map<int, ft_rarity_band> bands;

    table.set_rarity_bands(bands);
    return ;
}

static void operation_set_vendor_profiles(ft_economy_table &table)
{
    ft_map<int, ft_vendor_profile> profiles;

    table.set_vendor_profiles(profiles);
    return ;
}

static void operation_set_currency_rates(ft_economy_table &table)
{
    ft_map<int, ft_currency_rate> rates;

    table.set_currency_rates(rates);
    return ;
}

static void operation_enable_thread_safety(ft_economy_table &table)
{
    (void)table.enable_thread_safety();
    return ;
}

static void operation_is_thread_safe(ft_economy_table &table)
{
    (void)table.is_thread_safe();
    return ;
}

static void operation_lock(ft_economy_table &table)
{
    bool lock_acquired;

    lock_acquired = false;
    (void)table.lock(&lock_acquired);
    return ;
}

static void operation_unlock(ft_economy_table &table)
{
    table.unlock(false);
    return ;
}

FT_TEST(test_economy_lifecycle_register_price_uninitialized_aborts, "economy table lifecycle aborts for register price while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_register_price));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_rarity_uninitialized_aborts, "economy table lifecycle aborts for register rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_register_rarity));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_vendor_uninitialized_aborts, "economy table lifecycle aborts for register vendor while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_register_vendor));
    return (1);
}

FT_TEST(test_economy_lifecycle_register_currency_uninitialized_aborts, "economy table lifecycle aborts for register currency while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_register_currency));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_price_uninitialized_aborts, "economy table lifecycle aborts for fetch price while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_fetch_price));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_rarity_uninitialized_aborts, "economy table lifecycle aborts for fetch rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_fetch_rarity));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_vendor_uninitialized_aborts, "economy table lifecycle aborts for fetch vendor while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_fetch_vendor));
    return (1);
}

FT_TEST(test_economy_lifecycle_fetch_currency_uninitialized_aborts, "economy table lifecycle aborts for fetch currency while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_fetch_currency));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_price_definitions_uninitialized_aborts, "economy table lifecycle aborts for get price definitions while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_get_price_definitions));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_rarity_bands_uninitialized_aborts, "economy table lifecycle aborts for get rarity bands while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_get_rarity_bands));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_vendor_profiles_uninitialized_aborts, "economy table lifecycle aborts for get vendor profiles while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_get_vendor_profiles));
    return (1);
}

FT_TEST(test_economy_lifecycle_get_currency_rates_uninitialized_aborts, "economy table lifecycle aborts for get currency rates while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_get_currency_rates));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_price_definitions_uninitialized_aborts, "economy table lifecycle aborts for set price definitions while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_set_price_definitions));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_rarity_bands_uninitialized_aborts, "economy table lifecycle aborts for set rarity bands while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_set_rarity_bands));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_vendor_profiles_uninitialized_aborts, "economy table lifecycle aborts for set vendor profiles while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_set_vendor_profiles));
    return (1);
}

FT_TEST(test_economy_lifecycle_set_currency_rates_uninitialized_aborts, "economy table lifecycle aborts for set currency rates while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_set_currency_rates));
    return (1);
}

FT_TEST(test_economy_lifecycle_enable_thread_safety_uninitialized_aborts, "economy table lifecycle aborts for enable_thread_safety while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_enable_thread_safety));
    return (1);
}

FT_TEST(test_economy_lifecycle_is_thread_safe_uninitialized_aborts, "economy table lifecycle aborts for is_thread_safe while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_is_thread_safe));
    return (1);
}

FT_TEST(test_economy_lifecycle_lock_uninitialized_aborts, "economy table lifecycle aborts for lock while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_lock));
    return (1);
}

FT_TEST(test_economy_lifecycle_unlock_uninitialized_aborts, "economy table lifecycle aborts for unlock while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(operation_unlock));
    return (1);
}
