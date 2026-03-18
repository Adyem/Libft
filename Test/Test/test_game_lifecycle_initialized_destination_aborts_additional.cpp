#include "../test_internal.hpp"
#include "../../Game/game_price_definition.hpp"
#include "../../Game/game_rarity_band.hpp"
#include "../../Game/game_vendor_profile.hpp"
#include "../../Game/game_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_initialised_destination_signal_caught = 0;
static sigjmp_buf g_initialised_destination_jump_buffer;

static void initialised_destination_signal_handler(int signal_value)
{
    g_initialised_destination_signal_caught = signal_value;
    siglongjmp(g_initialised_destination_jump_buffer, 1);
    return ;
}

template <typename TypeName>
static int expect_sigabrt_copy_into_initialised_destination()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;
    int result;
    bool abort_handler_installed;
    bool iot_handler_installed;
    bool use_iot_signal;

    result = 0;
    abort_handler_installed = false;
    iot_handler_installed = false;
    use_iot_signal = (SIGIOT != SIGABRT);
    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialised_destination_signal_handler;
    new_action_iot.sa_handler = &initialised_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        goto cleanup;
    abort_handler_installed = true;
    if (use_iot_signal == true)
    {
        if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
            goto cleanup;
        iot_handler_installed = true;
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    g_initialised_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialised_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(source);
    if (g_initialised_destination_signal_caught == SIGABRT)
        result = 1;
    if (use_iot_signal == true && g_initialised_destination_signal_caught == SIGIOT)
        result = 1;
cleanup:
    if (iot_handler_installed == true)
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (abort_handler_installed == true)
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    return (result);
}

template <typename TypeName>
static int expect_sigabrt_move_into_initialised_destination()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;
    int result;
    bool abort_handler_installed;
    bool iot_handler_installed;
    bool use_iot_signal;

    result = 0;
    abort_handler_installed = false;
    iot_handler_installed = false;
    use_iot_signal = (SIGIOT != SIGABRT);
    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialised_destination_signal_handler;
    new_action_iot.sa_handler = &initialised_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        goto cleanup;
    abort_handler_installed = true;
    if (use_iot_signal == true)
    {
        if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
            goto cleanup;
        iot_handler_installed = true;
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    g_initialised_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialised_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(static_cast<TypeName &&>(source));
    if (g_initialised_destination_signal_caught == SIGABRT)
        result = 1;
    if (use_iot_signal == true && g_initialised_destination_signal_caught == SIGIOT)
        result = 1;
cleanup:
    if (iot_handler_installed == true)
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (abort_handler_installed == true)
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    return (result);
}

template <typename TypeName>
static int expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;
    int result;
    bool abort_handler_installed;
    bool iot_handler_installed;
    bool use_iot_signal;

    result = 0;
    abort_handler_installed = false;
    iot_handler_installed = false;
    use_iot_signal = (SIGIOT != SIGABRT);
    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialised_destination_signal_handler;
    new_action_iot.sa_handler = &initialised_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        goto cleanup;
    abort_handler_installed = true;
    if (use_iot_signal == true)
    {
        if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
            goto cleanup;
        iot_handler_installed = true;
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.destroy() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    g_initialised_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialised_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(source);
    if (g_initialised_destination_signal_caught == SIGABRT)
        result = 1;
    if (use_iot_signal == true && g_initialised_destination_signal_caught == SIGIOT)
        result = 1;
cleanup:
    if (iot_handler_installed == true)
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (abort_handler_installed == true)
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    return (result);
}

template <typename TypeName>
static int expect_move_from_initialised_source_into_destroyed_then_initialised_aborts()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;
    int result;
    bool abort_handler_installed;
    bool iot_handler_installed;
    bool use_iot_signal;

    result = 0;
    abort_handler_installed = false;
    iot_handler_installed = false;
    use_iot_signal = (SIGIOT != SIGABRT);
    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialised_destination_signal_handler;
    new_action_iot.sa_handler = &initialised_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        goto cleanup;
    abort_handler_installed = true;
    if (use_iot_signal == true)
    {
        if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
            goto cleanup;
        iot_handler_installed = true;
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.destroy() != FT_ERR_SUCCESS)
        goto cleanup;
    if (destination.initialize() != FT_ERR_SUCCESS)
        goto cleanup;
    g_initialised_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialised_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(static_cast<TypeName &&>(source));
    if (g_initialised_destination_signal_caught == SIGABRT)
        result = 1;
    if (use_iot_signal == true && g_initialised_destination_signal_caught == SIGIOT)
        result = 1;
cleanup:
    if (iot_handler_installed == true)
        (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (abort_handler_installed == true)
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    return (result);
}

FT_TEST(test_price_copy_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialised_destination<game_price_definition>());
    return (1);
}

FT_TEST(test_price_move_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialised_destination<game_price_definition>());
    return (1);
}

FT_TEST(test_price_copy_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts<game_price_definition>());
    return (1);
}

FT_TEST(test_price_move_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_move_from_initialised_source_into_destroyed_then_initialised_aborts<game_price_definition>());
    return (1);
}

FT_TEST(test_rarity_copy_into_initialised_destination_succeeds)
{
    FT_ASSERT_EQ(0, expect_sigabrt_copy_into_initialised_destination<game_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_into_initialised_destination_succeeds)
{
    FT_ASSERT_EQ(0, expect_sigabrt_move_into_initialised_destination<game_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_copy_into_reinitialised_destination_succeeds)
{
    FT_ASSERT_EQ(0, expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts<game_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_into_reinitialised_destination_succeeds)
{
    FT_ASSERT_EQ(0, expect_move_from_initialised_source_into_destroyed_then_initialised_aborts<game_rarity_band>());
    return (1);
}

FT_TEST(test_vendor_copy_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialised_destination<game_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialised_destination<game_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_copy_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts<game_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_move_from_initialised_source_into_destroyed_then_initialised_aborts<game_vendor_profile>());
    return (1);
}

FT_TEST(test_currency_copy_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialised_destination<game_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialised_destination<game_currency_rate>());
    return (1);
}

FT_TEST(test_currency_copy_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts<game_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_move_from_initialised_source_into_destroyed_then_initialised_aborts<game_currency_rate>());
    return (1);
}

FT_TEST(test_table_copy_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialised_destination<game_economy_table>());
    return (1);
}

FT_TEST(test_table_move_into_initialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialised_destination<game_economy_table>());
    return (1);
}

FT_TEST(test_table_copy_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_copy_from_initialised_source_into_destroyed_then_initialised_aborts<game_economy_table>());
    return (1);
}

FT_TEST(test_table_move_into_reinitialised_destination_aborts)
{
    FT_ASSERT_EQ(1, expect_move_from_initialised_source_into_destroyed_then_initialised_aborts<game_economy_table>());
    return (1);
}
