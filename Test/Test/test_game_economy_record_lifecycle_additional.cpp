#include "../test_internal.hpp"
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

static volatile sig_atomic_t g_record_lifecycle_signal_caught = 0;
static sigjmp_buf g_record_lifecycle_signal_jump_buffer;

static void record_lifecycle_signal_handler(int signal_value)
{
    g_record_lifecycle_signal_caught = signal_value;
    siglongjmp(g_record_lifecycle_signal_jump_buffer, 1);
    return ;
}

template <typename TypeName>
static int expect_sigabrt_on_uninitialized_object(void (*operation)(TypeName &))
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &record_lifecycle_signal_handler;
    new_action_iot.sa_handler = &record_lifecycle_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    g_record_lifecycle_signal_caught = 0;
    jump_result = sigsetjmp(g_record_lifecycle_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        alignas(TypeName) unsigned char storage[sizeof(TypeName)];
        TypeName *object_pointer;

        std::memset(storage, 0, sizeof(storage));
        object_pointer = reinterpret_cast<TypeName *>(storage);
        operation(*object_pointer);
    }
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_record_lifecycle_signal_caught == SIGABRT)
        return (1);
    return (g_record_lifecycle_signal_caught == SIGIOT);
}

static void price_get_item_id(ft_price_definition &definition) { (void)definition.get_item_id(); return ; }
static void price_get_rarity(ft_price_definition &definition) { (void)definition.get_rarity(); return ; }
static void price_get_base_value(ft_price_definition &definition) { (void)definition.get_base_value(); return ; }
static void price_set_item_id(ft_price_definition &definition) { definition.set_item_id(10); return ; }
static void price_set_rarity(ft_price_definition &definition) { definition.set_rarity(3); return ; }

static void rarity_get_rarity(ft_rarity_band &band) { (void)band.get_rarity(); return ; }
static void rarity_get_value_multiplier(ft_rarity_band &band) { (void)band.get_value_multiplier(); return ; }
static void rarity_set_rarity(ft_rarity_band &band) { band.set_rarity(2); return ; }
static void rarity_set_value_multiplier(ft_rarity_band &band) { band.set_value_multiplier(1.5); return ; }
static void rarity_is_thread_safe(ft_rarity_band &band) { (void)band.is_thread_safe(); return ; }

static void vendor_get_vendor_id(ft_vendor_profile &profile) { (void)profile.get_vendor_id(); return ; }
static void vendor_get_buy_markup(ft_vendor_profile &profile) { (void)profile.get_buy_markup(); return ; }
static void vendor_set_vendor_id(ft_vendor_profile &profile) { profile.set_vendor_id(4); return ; }
static void vendor_set_buy_markup(ft_vendor_profile &profile) { profile.set_buy_markup(1.2); return ; }
static void vendor_enable_thread_safety(ft_vendor_profile &profile) { (void)profile.enable_thread_safety(); return ; }

static void currency_get_currency_id(ft_currency_rate &rate) { (void)rate.get_currency_id(); return ; }
static void currency_get_rate_to_base(ft_currency_rate &rate) { (void)rate.get_rate_to_base(); return ; }
static void currency_get_display_precision(ft_currency_rate &rate) { (void)rate.get_display_precision(); return ; }
static void currency_set_currency_id(ft_currency_rate &rate) { rate.set_currency_id(5); return ; }
static void currency_set_rate_to_base(ft_currency_rate &rate) { rate.set_rate_to_base(2.0); return ; }

FT_TEST(test_price_lifecycle_get_item_id_uninitialized_aborts, "price definition lifecycle aborts for get_item_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(price_get_item_id));
    return (1);
}

FT_TEST(test_price_lifecycle_get_rarity_uninitialized_aborts, "price definition lifecycle aborts for get_rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(price_get_rarity));
    return (1);
}

FT_TEST(test_price_lifecycle_get_base_value_uninitialized_aborts, "price definition lifecycle aborts for get_base_value while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(price_get_base_value));
    return (1);
}

FT_TEST(test_price_lifecycle_set_item_id_uninitialized_aborts, "price definition lifecycle aborts for set_item_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(price_set_item_id));
    return (1);
}

FT_TEST(test_price_lifecycle_set_rarity_uninitialized_aborts, "price definition lifecycle aborts for set_rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(price_set_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_get_rarity_uninitialized_aborts, "rarity band lifecycle aborts for get_rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(rarity_get_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_get_value_multiplier_uninitialized_aborts, "rarity band lifecycle aborts for get_value_multiplier while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(rarity_get_value_multiplier));
    return (1);
}

FT_TEST(test_rarity_lifecycle_set_rarity_uninitialized_aborts, "rarity band lifecycle aborts for set_rarity while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(rarity_set_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_set_value_multiplier_uninitialized_aborts, "rarity band lifecycle aborts for set_value_multiplier while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(rarity_set_value_multiplier));
    return (1);
}

FT_TEST(test_rarity_lifecycle_is_thread_safe_uninitialized_aborts, "rarity band lifecycle aborts for is_thread_safe while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(rarity_is_thread_safe));
    return (1);
}

FT_TEST(test_vendor_lifecycle_get_vendor_id_uninitialized_aborts, "vendor profile lifecycle aborts for get_vendor_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(vendor_get_vendor_id));
    return (1);
}

FT_TEST(test_vendor_lifecycle_get_buy_markup_uninitialized_aborts, "vendor profile lifecycle aborts for get_buy_markup while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(vendor_get_buy_markup));
    return (1);
}

FT_TEST(test_vendor_lifecycle_set_vendor_id_uninitialized_aborts, "vendor profile lifecycle aborts for set_vendor_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(vendor_set_vendor_id));
    return (1);
}

FT_TEST(test_vendor_lifecycle_set_buy_markup_uninitialized_aborts, "vendor profile lifecycle aborts for set_buy_markup while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(vendor_set_buy_markup));
    return (1);
}

FT_TEST(test_vendor_lifecycle_enable_thread_safety_uninitialized_aborts, "vendor profile lifecycle aborts for enable_thread_safety while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(vendor_enable_thread_safety));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_currency_id_uninitialized_aborts, "currency rate lifecycle aborts for get_currency_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(currency_get_currency_id));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_rate_to_base_uninitialized_aborts, "currency rate lifecycle aborts for get_rate_to_base while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(currency_get_rate_to_base));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_display_precision_uninitialized_aborts, "currency rate lifecycle aborts for get_display_precision while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(currency_get_display_precision));
    return (1);
}

FT_TEST(test_currency_lifecycle_set_currency_id_uninitialized_aborts, "currency rate lifecycle aborts for set_currency_id while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(currency_set_currency_id));
    return (1);
}

FT_TEST(test_currency_lifecycle_set_rate_to_base_uninitialized_aborts, "currency rate lifecycle aborts for set_rate_to_base while uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialized_object(currency_set_rate_to_base));
    return (1);
}
