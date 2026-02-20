#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

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

static void economy_table_signal_handler(int signal_value)
{
    g_economy_table_signal_caught = signal_value;
    siglongjmp(g_economy_table_signal_jump_buffer, 1);
}

static int economy_table_expect_sigabrt_uninitialized(void (*operation)(ft_economy_table &))
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &economy_table_signal_handler;
    new_action_iot.sa_handler = &economy_table_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }

    g_economy_table_signal_caught = 0;
    jump_result = sigsetjmp(g_economy_table_signal_jump_buffer, 1);
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
    if (g_economy_table_signal_caught == SIGABRT)
        return (1);
    return (g_economy_table_signal_caught == SIGIOT);
}

static void economy_table_call_copy_from_uninitialized(ft_economy_table &source)
{
    ft_economy_table destination;

    (void)destination.initialize(source);
    return ;
}

static void economy_table_call_move_from_uninitialized(ft_economy_table &source)
{
    ft_economy_table destination;

    (void)destination.initialize(ft_move(source));
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
