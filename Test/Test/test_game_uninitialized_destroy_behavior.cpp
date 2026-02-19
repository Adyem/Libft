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

static volatile sig_atomic_t g_signal_caught = 0;
static sigjmp_buf g_signal_jump_buffer;

static void uninitialized_destructor_signal_handler(int signal_value)
{
    g_signal_caught = signal_value;
    siglongjmp(g_signal_jump_buffer, 1);
}

template <typename TypeName>
static int expect_no_sigabrt_on_uninitialized_destructor()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &uninitialized_destructor_signal_handler;
    new_action_iot.sa_handler = &uninitialized_destructor_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }

    g_signal_caught = 0;
    jump_result = sigsetjmp(g_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        alignas(TypeName) unsigned char storage[sizeof(TypeName)];
        TypeName *object_pointer;

        std::memset(storage, 0, sizeof(storage));
        object_pointer = reinterpret_cast<TypeName *>(storage);
        object_pointer->~TypeName();
    }

    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    return (g_signal_caught == 0);
}

FT_TEST(test_game_price_definition_destroy_uninitialized_returns_invalid_state,
    "ft_price_definition destroy on uninitialized object is non-aborting")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.destroy());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_price_definition>());
    return (1);
}

FT_TEST(test_game_rarity_band_destroy_uninitialized_returns_invalid_state,
    "ft_rarity_band destroy on uninitialized object is non-aborting")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.destroy());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_rarity_band>());
    return (1);
}

FT_TEST(test_game_vendor_profile_destroy_uninitialized_returns_invalid_state,
    "ft_vendor_profile destroy on uninitialized object is non-aborting")
{
    ft_vendor_profile vendor;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, vendor.destroy());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_game_currency_rate_destroy_uninitialized_returns_invalid_state,
    "ft_currency_rate destroy on uninitialized object is non-aborting")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.destroy());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_currency_rate>());
    return (1);
}

FT_TEST(test_game_economy_table_destroy_uninitialized_returns_invalid_state,
    "ft_economy_table destroy on uninitialized object is non-aborting")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_economy_table>());
    return (1);
}
