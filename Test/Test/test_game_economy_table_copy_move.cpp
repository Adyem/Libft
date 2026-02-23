#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static int register_all_records(ft_economy_table &table)
{
    ft_price_definition definition;
    ft_rarity_band band;
    ft_vendor_profile vendor;
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(4, 2, 900, 400, 1200));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, band.initialize(7, 1.15));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor.initialize(11, 1.30, 0.55, 0.08));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rate.initialize(5, 2.50, 4));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_price_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_rarity_band(band));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_vendor_profile(vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_currency_rate(rate));
    return (1);
}

static volatile sig_atomic_t g_economy_table_signal_caught = 0;
static sigjmp_buf g_economy_table_signal_jump_buffer;
static struct sigaction g_economy_table_old_action_abort;
static struct sigaction g_economy_table_old_action_iot;
static struct sigaction g_economy_table_new_action_abort;
static struct sigaction g_economy_table_new_action_iot;
static void *g_economy_table_uninitialized_storage = ft_nullptr;
static sigset_t g_economy_table_signal_mask;
static int g_economy_table_uninitialized_operation_error = FT_ERR_SUCCESS;

static void economy_table_signal_handler(int signal_value)
{
    g_economy_table_signal_caught = signal_value;
    siglongjmp(g_economy_table_signal_jump_buffer, 1);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((no_stack_protector))
#endif

static int economy_table_expect_sigabrt_uninitialized(void (*operation)(ft_economy_table &))
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
    {
        perror("sigaction SIGABRT failed");
        return (0);
    }
    if (SIGIOT != SIGABRT
        && sigaction(SIGIOT, &g_economy_table_new_action_iot,
            &g_economy_table_old_action_iot) != 0)
    {
        perror("sigaction SIGIOT failed");
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

        table_pointer = new (g_economy_table_uninitialized_storage) ft_economy_table();
        table_pointer->initialize();
        operation(*table_pointer);
        table_pointer->~ft_economy_table();
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

static void economy_table_call_copy_from_uninitialized(ft_economy_table &source)
{
    void *storage;
    ft_economy_table *destination;
    int result;

    storage = std::malloc(sizeof(ft_economy_table));
    if (storage == ft_nullptr)
    {
        g_economy_table_uninitialized_operation_error = FT_ERR_NO_MEMORY;
        return ;
    }
    destination = new (storage) ft_economy_table();
    result = destination->initialize(source);
    g_economy_table_uninitialized_operation_error = result;
    destination->~ft_economy_table();
    std::free(storage);
    return ;
}

static void economy_table_call_move_from_uninitialized(ft_economy_table &source)
{
    void *storage;
    ft_economy_table *destination;
    int result;

    storage = std::malloc(sizeof(ft_economy_table));
    if (storage == ft_nullptr)
    {
        g_economy_table_uninitialized_operation_error = FT_ERR_NO_MEMORY;
        return ;
    }
    destination = new (storage) ft_economy_table();
    result = destination->initialize(ft_move(source));
    g_economy_table_uninitialized_operation_error = result;
    destination->~ft_economy_table();
    std::free(storage);
    return ;
}

FT_TEST(test_economy_table_initialize_copy, "economy table initialize(copy) duplicates entries")
{
    ft_economy_table source;
    ft_economy_table duplicate;
    ft_price_definition price;
    ft_price_definition source_price;
    ft_rarity_band rarity;
    ft_rarity_band source_rarity;
    ft_vendor_profile vendor;
    ft_vendor_profile source_vendor;
    ft_currency_rate currency;
    ft_currency_rate source_currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(source));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, price.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_currency_rate(5, currency));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_price.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_rarity.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_price_definition(4, source_price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_rarity_band(7, source_rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_vendor_profile(11, source_vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_currency_rate(5, source_currency));
    return (1);
}

FT_TEST(test_economy_table_initialize_copy_uninitialized_destination,
        "economy table initialize(copy) works even when destination starts uninitialized")
{
    ft_economy_table source;
    ft_economy_table duplicate;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.initialize(source));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, price.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.fetch_currency_rate(5, currency));
    return (1);
}

FT_TEST(test_economy_table_initialize_overwrites_existing_entries, "economy table initialize(copy) replaces current entries")
{
    ft_economy_table source;
    ft_economy_table destination;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    register_all_records(destination);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, price.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_currency_rate(5, currency));
    return (1);
}

FT_TEST(test_economy_table_initialize_move_transfers_entries, "economy table initialize(move) transfers entries and clears source")
{
    ft_economy_table source;
    ft_economy_table moved;
    ft_price_definition price;
    ft_rarity_band rarity;
    ft_vendor_profile vendor;
    ft_currency_rate currency;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    register_all_records(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(ft_move(source)));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, price.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, currency.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_price_definition(4, price));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_rarity_band(7, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_vendor_profile(11, vendor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_currency_rate(5, currency));
    FT_ASSERT(source.get_price_definitions().empty());
    FT_ASSERT(source.get_rarity_bands().empty());
    FT_ASSERT(source.get_vendor_profiles().empty());
    FT_ASSERT(source.get_currency_rates().empty());
    return (1);
}

FT_TEST(test_economy_table_initialize_copy_uninitialized_source_aborts,
        "economy table initialize(copy) aborts when source is uninitialized")
{
    FT_ASSERT_EQ(1, economy_table_expect_sigabrt_uninitialized(
        economy_table_call_copy_from_uninitialized));
    return (1);
}

FT_TEST(test_economy_table_initialize_move_uninitialized_source_aborts,
        "economy table initialize(move) aborts when source is uninitialized")
{
    FT_ASSERT_EQ(1, economy_table_expect_sigabrt_uninitialized(
        economy_table_call_move_from_uninitialized));
    return (1);
}
