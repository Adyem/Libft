#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_economy_table_signal_caught = 0;
static sigjmp_buf g_economy_table_signal_jump_buffer;
static struct sigaction g_economy_table_old_action_abort;
static struct sigaction g_economy_table_old_action_iot;
static struct sigaction g_economy_table_new_action_abort;
static struct sigaction g_economy_table_new_action_iot;
static sigset_t g_economy_table_signal_mask;
static void *g_economy_table_uninitialized_storage = ft_nullptr;
static int g_economy_table_uninitialized_operation_error = FT_ERR_SUCCESS;

static void economy_table_signal_handler(int signal_value)
{
    g_economy_table_signal_caught = signal_value;
    siglongjmp(g_economy_table_signal_jump_buffer, 1);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((no_stack_protector))
#endif
static int expect_sigabrt_on_uninitialized_table(void (*operation)(ft_economy_table &))
{
    std::memset(&g_economy_table_old_action_abort, 0,
        sizeof(g_economy_table_old_action_abort));
    std::memset(&g_economy_table_old_action_iot, 0,
        sizeof(g_economy_table_old_action_iot));
    std::memset(&g_economy_table_new_action_abort, 0,
        sizeof(g_economy_table_new_action_abort));
    std::memset(&g_economy_table_new_action_iot, 0,
        sizeof(g_economy_table_new_action_iot));
    g_economy_table_new_action_abort.sa_handler = &economy_table_signal_handler;
    g_economy_table_new_action_iot.sa_handler = &economy_table_signal_handler;
    sigemptyset(&g_economy_table_new_action_abort.sa_mask);
    sigemptyset(&g_economy_table_new_action_iot.sa_mask);
    sigemptyset(&g_economy_table_signal_mask);
    sigaddset(&g_economy_table_signal_mask, SIGABRT);
    sigaddset(&g_economy_table_signal_mask, SIGIOT);
    if (pthread_sigmask(SIG_UNBLOCK, &g_economy_table_signal_mask, ft_nullptr) != 0)
    {
    }
    if (sigaction(SIGABRT, &g_economy_table_new_action_abort,
            &g_economy_table_old_action_abort) != 0)
        return (0);
    if (SIGIOT != SIGABRT
        && sigaction(SIGIOT, &g_economy_table_new_action_iot,
            &g_economy_table_old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &g_economy_table_old_action_abort, ft_nullptr);
        return (0);
    }
    g_economy_table_uninitialized_storage = std::malloc(sizeof(ft_economy_table));
    if (g_economy_table_uninitialized_storage == ft_nullptr)
    {
        (void)sigaction(SIGABRT, &g_economy_table_old_action_abort, ft_nullptr);
        if (SIGIOT != SIGABRT)
            (void)sigaction(SIGIOT, &g_economy_table_old_action_iot, ft_nullptr);
        return (0);
    }
    g_economy_table_signal_caught = 0;
    g_economy_table_uninitialized_operation_error = FT_ERR_SUCCESS;
    if (sigsetjmp(g_economy_table_signal_jump_buffer, 1) == 0)
    {
        ft_economy_table *table_pointer;

        std::memset(g_economy_table_uninitialized_storage, 0,
            sizeof(ft_economy_table));
        table_pointer = reinterpret_cast<ft_economy_table *>(
            g_economy_table_uninitialized_storage);
        operation(*table_pointer);
    }
    if (g_economy_table_uninitialized_storage != ft_nullptr)
    {
        std::free(g_economy_table_uninitialized_storage);
        g_economy_table_uninitialized_storage = ft_nullptr;
    }
    (void)sigaction(SIGABRT, &g_economy_table_old_action_abort, ft_nullptr);
    if (SIGIOT != SIGABRT)
        (void)sigaction(SIGIOT, &g_economy_table_old_action_iot, ft_nullptr);
    if (g_economy_table_signal_caught == SIGABRT)
        return (1);
    if (g_economy_table_signal_caught == SIGIOT)
        return (1);
    return (g_economy_table_uninitialized_operation_error == FT_ERR_SUCCESS);
}

static void uninitialized_table_register_price_operation(ft_economy_table &table)
{
    void *storage;
    ft_price_definition *definition;
    int result;

    storage = std::malloc(sizeof(ft_price_definition));
    if (storage == ft_nullptr)
    {
        g_economy_table_uninitialized_operation_error = FT_ERR_NO_MEMORY;
        return ;
    }
    definition = new (storage) ft_price_definition();
    result = definition->initialize(10, 3, 500, 300, 800);
    if (result != FT_ERR_SUCCESS)
    {
        g_economy_table_uninitialized_operation_error = result;
        definition->~ft_price_definition();
        std::free(storage);
        return ;
    }
    g_economy_table_uninitialized_operation_error =
        table.register_price_definition(*definition);
    definition->~ft_price_definition();
    std::free(storage);
    return ;
}

static void uninitialized_table_fetch_price_operation(ft_economy_table &table)
{
    void *storage;
    ft_price_definition *fetched;
    int result;

    storage = std::malloc(sizeof(ft_price_definition));
    if (storage == ft_nullptr)
    {
        g_economy_table_uninitialized_operation_error = FT_ERR_NO_MEMORY;
        return ;
    }
    fetched = new (storage) ft_price_definition();
    result = fetched->initialize();
    if (result != FT_ERR_SUCCESS)
    {
        g_economy_table_uninitialized_operation_error = result;
        fetched->~ft_price_definition();
        std::free(storage);
        return ;
    }
    g_economy_table_uninitialized_operation_error =
        table.fetch_price_definition(10, *fetched);
    fetched->~ft_price_definition();
    std::free(storage);
    return ;
}

static int assert_price_definition(const ft_price_definition &definition,
        int item_id, int rarity, int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    return (1);
}

static int assert_vendor_profile_values(const ft_vendor_profile &profile,
        int vendor_id, double buy_markup, double sell_multiplier, double tax_rate)
{
    FT_ASSERT_EQ(vendor_id, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(buy_markup, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(sell_multiplier, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(tax_rate, profile.get_tax_rate());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_economy_register_and_fetch_price, "register and fetch price definition")
{
    ft_economy_table table;
    ft_price_definition definition;
    ft_price_definition fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(10, 3, 500, 300, 800));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_price_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_price_definition(10, fetched));
    assert_price_definition(fetched, 10, 3, 500, 300, 800);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_economy_missing_entries, "missing entries return not found")
{
    ft_economy_table table;
    ft_price_definition definition;
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_price_definition(99, definition));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_currency_rate(7, rate));
    return (1);
}

FT_TEST(test_economy_rarity_band_registration, "register and fetch rarity band")
{
    ft_economy_table table;
    ft_rarity_band band;
    ft_rarity_band fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(4, 1.75));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_rarity_band(band));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_rarity_band(4, fetched));
    FT_ASSERT_EQ(4, fetched.get_rarity());
    FT_ASSERT_DOUBLE_EQ(1.75, fetched.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_economy_vendor_and_currency_profiles,
        "register vendor and currency profiles")
{
    ft_economy_table table;
    ft_vendor_profile vendor;
    ft_currency_rate currency;
    ft_vendor_profile fetched_vendor;
    ft_currency_rate fetched_currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor.initialize(2, 1.25, 0.60, 0.05));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize(1, 0.5, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_currency_rate(currency));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_vendor_profile(2, fetched_vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_currency_rate(1, fetched_currency));
    assert_vendor_profile_values(fetched_vendor, 2, 1.25, 0.60, 0.05);
    FT_ASSERT_EQ(1, fetched_currency.get_currency_id());
    FT_ASSERT_DOUBLE_EQ(0.5, fetched_currency.get_rate_to_base());
    FT_ASSERT_EQ(3, fetched_currency.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_vendor.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_currency.get_error());
    return (1);
}

FT_TEST(test_economy_register_price_uninitialized_aborts,
    "economy table register aborts when table is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(
        uninitialized_table_register_price_operation));
    return (1);
}

FT_TEST(test_economy_fetch_price_uninitialized_aborts,
    "economy table fetch aborts when table is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_table(
        uninitialized_table_fetch_price_operation));
    return (1);
}
