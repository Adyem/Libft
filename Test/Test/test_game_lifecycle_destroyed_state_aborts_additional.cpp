#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_destroyed_state_signal_caught = 0;
static sigjmp_buf g_destroyed_state_jump_buffer;

static void destroyed_state_signal_handler(int signal_value)
{
    g_destroyed_state_signal_caught = signal_value;
    siglongjmp(g_destroyed_state_jump_buffer, 1);
    return ;
}

template <typename TypeName>
static int expect_sigabrt_on_destroyed_object(void (*operation)(TypeName &))
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName object_instance;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &destroyed_state_signal_handler;
    new_action_iot.sa_handler = &destroyed_state_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    if (object_instance.initialize() != FT_ERR_SUCCESS)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
        return (0);
    }
    if (object_instance.destroy() != FT_ERR_SUCCESS)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
        return (0);
    }

    g_destroyed_state_signal_caught = 0;
    jump_result = sigsetjmp(g_destroyed_state_jump_buffer, 1);
    if (jump_result == 0)
        operation(object_instance);

    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_destroyed_state_signal_caught == SIGABRT)
        return (1);
    return (g_destroyed_state_signal_caught == SIGIOT);
}

static void price_destroyed_get_item_id(ft_price_definition &definition) { (void)definition.get_item_id(); return ; }
static void price_destroyed_set_item_id(ft_price_definition &definition) { definition.set_item_id(8); return ; }
static void price_destroyed_enable_thread_safety(ft_price_definition &definition) { (void)definition.enable_thread_safety(); return ; }
static void price_destroyed_lock(ft_price_definition &definition) { bool lock_acquired = false; (void)definition.lock(&lock_acquired); return ; }

static void rarity_destroyed_get_rarity(ft_rarity_band &band) { (void)band.get_rarity(); return ; }
static void rarity_destroyed_set_rarity(ft_rarity_band &band) { band.set_rarity(3); return ; }
static void rarity_destroyed_enable_thread_safety(ft_rarity_band &band) { (void)band.enable_thread_safety(); return ; }
static void rarity_destroyed_lock(ft_rarity_band &band) { bool lock_acquired = false; (void)band.lock(&lock_acquired); return ; }

static void vendor_destroyed_get_vendor_id(ft_vendor_profile &profile) { (void)profile.get_vendor_id(); return ; }
static void vendor_destroyed_set_vendor_id(ft_vendor_profile &profile) { profile.set_vendor_id(5); return ; }
static void vendor_destroyed_enable_thread_safety(ft_vendor_profile &profile) { (void)profile.enable_thread_safety(); return ; }
static void vendor_destroyed_lock(ft_vendor_profile &profile) { bool lock_acquired = false; (void)profile.lock(&lock_acquired); return ; }

static void currency_destroyed_get_currency_id(ft_currency_rate &rate) { (void)rate.get_currency_id(); return ; }
static void currency_destroyed_set_currency_id(ft_currency_rate &rate) { rate.set_currency_id(7); return ; }
static void currency_destroyed_enable_thread_safety(ft_currency_rate &rate) { (void)rate.enable_thread_safety(); return ; }
static void currency_destroyed_lock(ft_currency_rate &rate) { bool lock_acquired = false; (void)rate.lock(&lock_acquired); return ; }

static void table_destroyed_get_price_definitions(ft_economy_table &table) { (void)table.get_price_definitions(); return ; }
static void table_destroyed_register_price(ft_economy_table &table) { ft_price_definition definition; (void)definition.initialize(); (void)table.register_price_definition(definition); return ; }
static void table_destroyed_enable_thread_safety(ft_economy_table &table) { (void)table.enable_thread_safety(); return ; }
static void table_destroyed_lock(ft_economy_table &table) { bool lock_acquired = false; (void)table.lock(&lock_acquired); return ; }

FT_TEST(test_price_destroyed_get_item_id_aborts, "price definition destroyed state aborts get_item_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(price_destroyed_get_item_id));
    return (1);
}

FT_TEST(test_price_destroyed_set_item_id_aborts, "price definition destroyed state aborts set_item_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(price_destroyed_set_item_id));
    return (1);
}

FT_TEST(test_price_destroyed_enable_thread_safety_aborts, "price definition destroyed state aborts enable_thread_safety")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(price_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_price_destroyed_lock_aborts, "price definition destroyed state aborts lock")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(price_destroyed_lock));
    return (1);
}

FT_TEST(test_rarity_destroyed_get_rarity_aborts, "rarity band destroyed state aborts get_rarity")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(rarity_destroyed_get_rarity));
    return (1);
}

FT_TEST(test_rarity_destroyed_set_rarity_aborts, "rarity band destroyed state aborts set_rarity")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(rarity_destroyed_set_rarity));
    return (1);
}

FT_TEST(test_rarity_destroyed_enable_thread_safety_aborts, "rarity band destroyed state aborts enable_thread_safety")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(rarity_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_rarity_destroyed_lock_aborts, "rarity band destroyed state aborts lock")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(rarity_destroyed_lock));
    return (1);
}

FT_TEST(test_vendor_destroyed_get_vendor_id_aborts, "vendor profile destroyed state aborts get_vendor_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(vendor_destroyed_get_vendor_id));
    return (1);
}

FT_TEST(test_vendor_destroyed_set_vendor_id_aborts, "vendor profile destroyed state aborts set_vendor_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(vendor_destroyed_set_vendor_id));
    return (1);
}

FT_TEST(test_vendor_destroyed_enable_thread_safety_aborts, "vendor profile destroyed state aborts enable_thread_safety")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(vendor_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_vendor_destroyed_lock_aborts, "vendor profile destroyed state aborts lock")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(vendor_destroyed_lock));
    return (1);
}

FT_TEST(test_currency_destroyed_get_currency_id_aborts, "currency rate destroyed state aborts get_currency_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(currency_destroyed_get_currency_id));
    return (1);
}

FT_TEST(test_currency_destroyed_set_currency_id_aborts, "currency rate destroyed state aborts set_currency_id")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(currency_destroyed_set_currency_id));
    return (1);
}

FT_TEST(test_currency_destroyed_enable_thread_safety_aborts, "currency rate destroyed state aborts enable_thread_safety")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(currency_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_currency_destroyed_lock_aborts, "currency rate destroyed state aborts lock")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(currency_destroyed_lock));
    return (1);
}

FT_TEST(test_economy_table_destroyed_get_price_definitions_aborts, "economy table destroyed state aborts get_price_definitions")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(table_destroyed_get_price_definitions));
    return (1);
}

FT_TEST(test_economy_table_destroyed_register_price_aborts, "economy table destroyed state aborts register_price_definition")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(table_destroyed_register_price));
    return (1);
}

FT_TEST(test_economy_table_destroyed_enable_thread_safety_aborts, "economy table destroyed state aborts enable_thread_safety")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(table_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_economy_table_destroyed_lock_aborts, "economy table destroyed state aborts lock")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_destroyed_object(table_destroyed_lock));
    return (1);
}
