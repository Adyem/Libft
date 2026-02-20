#include "../test_internal.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_economy_table_signal_caught = 0;
static sigjmp_buf g_economy_table_signal_jump_buffer;

static void economy_table_signal_handler(int signal_value)
{
    g_economy_table_signal_caught = signal_value;
    siglongjmp(g_economy_table_signal_jump_buffer, 1);
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

static void uninitialized_table_register_price_operation(ft_economy_table &table)
{
    ft_price_definition definition;

    (void)definition.initialize(10, 3, 500, 300, 800);
    (void)table.register_price_definition(definition);
    return ;
}

static void uninitialized_table_fetch_price_operation(ft_economy_table &table)
{
    ft_price_definition fetched;

    (void)fetched.initialize();
    (void)table.fetch_price_definition(10, fetched);
    return ;
}

static int assert_price_definition(const ft_price_definition &definition, int item_id, int rarity,
        int base_value, int minimum_value, int maximum_value)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(base_value, definition.get_base_value());
    FT_ASSERT_EQ(minimum_value, definition.get_minimum_value());
    FT_ASSERT_EQ(maximum_value, definition.get_maximum_value());
    return (1);
}

static int assert_vendor_profile_values(const ft_vendor_profile &profile, int vendor_id,
        double buy_markup, double sell_multiplier, double tax_rate)
{
    FT_ASSERT_EQ(vendor_id, profile.get_vendor_id());
    FT_ASSERT_DOUBLE_EQ(buy_markup, profile.get_buy_markup());
    FT_ASSERT_DOUBLE_EQ(sell_multiplier, profile.get_sell_multiplier());
    FT_ASSERT_DOUBLE_EQ(tax_rate, profile.get_tax_rate());
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
    return (1);
}

FT_TEST(test_economy_vendor_and_currency_profiles, "register vendor and currency profiles")
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
