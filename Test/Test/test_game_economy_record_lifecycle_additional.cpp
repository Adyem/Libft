#include "../test_internal.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
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
static int expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;
    bool iot_handler_installed;
    int restore_error;
    static TypeName *object_pointer = ft_nullptr;

    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &record_lifecycle_signal_handler;
    new_action_iot.sa_handler = &record_lifecycle_signal_handler;
    if (sigemptyset(&new_action_abort.sa_mask) != 0)
        return (0);
    if (sigemptyset(&new_action_iot.sa_mask) != 0)
        return (0);
    iot_handler_installed = false;
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (SIGIOT != SIGABRT)
    {
        if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
        {
            if (sigaction(SIGABRT, &old_action_abort, ft_nullptr) != 0)
                return (0);
            return (0);
        }
        iot_handler_installed = true;
    }
    g_record_lifecycle_signal_caught = 0;
    jump_result = sigsetjmp(g_record_lifecycle_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        object_pointer = new (std::nothrow) TypeName();
        if (object_pointer == ft_nullptr)
        {
            if (sigaction(SIGABRT, &old_action_abort, ft_nullptr) != 0)
                return (0);
            if (iot_handler_installed == true)
            {
                if (sigaction(SIGIOT, &old_action_iot, ft_nullptr) != 0)
                    return (0);
            }
            return (0);
        }
        operation(*object_pointer);
        delete object_pointer;
        object_pointer = ft_nullptr;
    }
    if (object_pointer != ft_nullptr)
    {
        delete object_pointer;
        object_pointer = ft_nullptr;
    }
    restore_error = sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    if (restore_error != 0)
        return (0);
    if (iot_handler_installed == true)
    {
        restore_error = sigaction(SIGIOT, &old_action_iot, ft_nullptr);
        if (restore_error != 0)
            return (0);
    }
    if (g_record_lifecycle_signal_caught == SIGABRT)
        return (1);
    return (g_record_lifecycle_signal_caught == SIGIOT);
}

static void price_get_item_id(game_price_definition &definition) { (void)definition.get_item_id(); return ; }
static void price_get_rarity(game_price_definition &definition) { (void)definition.get_rarity(); return ; }
static void price_get_base_value(game_price_definition &definition) { (void)definition.get_base_value(); return ; }
static void price_set_item_id(game_price_definition &definition) { definition.set_item_id(10); return ; }
static void price_set_rarity(game_price_definition &definition) { definition.set_rarity(3); return ; }

static void rarity_get_rarity(game_rarity_band &band) { (void)band.get_rarity(); return ; }
static void rarity_get_value_multiplier(game_rarity_band &band) { (void)band.get_value_multiplier(); return ; }
static void rarity_set_rarity(game_rarity_band &band) { band.set_rarity(2); return ; }
static void rarity_set_value_multiplier(game_rarity_band &band) { band.set_value_multiplier(1.5); return ; }
static void rarity_is_thread_safe(game_rarity_band &band) { (void)band.is_thread_safe(); return ; }

static void vendor_get_vendor_id(game_vendor_profile &profile) { (void)profile.get_vendor_id(); return ; }
static void vendor_get_buy_markup(game_vendor_profile &profile) { (void)profile.get_buy_markup(); return ; }
static void vendor_set_vendor_id(game_vendor_profile &profile) { profile.set_vendor_id(4); return ; }
static void vendor_set_buy_markup(game_vendor_profile &profile) { profile.set_buy_markup(1.2); return ; }
static void vendor_enable_thread_safety(game_vendor_profile &profile) { (void)profile.enable_thread_safety(); return ; }

static void currency_get_currency_id(game_currency_rate &rate) { (void)rate.get_currency_id(); return ; }
static void currency_get_rate_to_base(game_currency_rate &rate) { (void)rate.get_rate_to_base(); return ; }
static void currency_get_display_precision(game_currency_rate &rate) { (void)rate.get_display_precision(); return ; }
static void currency_set_currency_id(game_currency_rate &rate) { rate.set_currency_id(5); return ; }
static void currency_set_rate_to_base(game_currency_rate &rate) { rate.set_rate_to_base(2.0); return ; }

FT_TEST(test_price_lifecycle_get_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(price_get_item_id));
    return (1);
}

FT_TEST(test_price_lifecycle_get_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(price_get_rarity));
    return (1);
}

FT_TEST(test_price_lifecycle_get_base_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(price_get_base_value));
    return (1);
}

FT_TEST(test_price_lifecycle_set_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(price_set_item_id));
    return (1);
}

FT_TEST(test_price_lifecycle_set_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(price_set_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_get_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(rarity_get_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_get_value_multiplier_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(rarity_get_value_multiplier));
    return (1);
}

FT_TEST(test_rarity_lifecycle_set_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(rarity_set_rarity));
    return (1);
}

FT_TEST(test_rarity_lifecycle_set_value_multiplier_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(rarity_set_value_multiplier));
    return (1);
}

FT_TEST(test_rarity_lifecycle_is_thread_safe_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object(rarity_is_thread_safe));
    return (1);
}

FT_TEST(test_vendor_lifecycle_get_vendor_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(vendor_get_vendor_id));
    return (1);
}

FT_TEST(test_vendor_lifecycle_get_buy_markup_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(vendor_get_buy_markup));
    return (1);
}

FT_TEST(test_vendor_lifecycle_set_vendor_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(vendor_set_vendor_id));
    return (1);
}

FT_TEST(test_vendor_lifecycle_set_buy_markup_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(vendor_set_buy_markup));
    return (1);
}

FT_TEST(test_vendor_lifecycle_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(vendor_enable_thread_safety));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_currency_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(currency_get_currency_id));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_rate_to_base_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(currency_get_rate_to_base));
    return (1);
}

FT_TEST(test_currency_lifecycle_get_display_precision_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(currency_get_display_precision));
    return (1);
}

FT_TEST(test_currency_lifecycle_set_currency_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(currency_set_currency_id));
    return (1);
}

FT_TEST(test_currency_lifecycle_set_rate_to_base_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object(currency_set_rate_to_base));
    return (1);
}
