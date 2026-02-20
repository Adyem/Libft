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

static volatile sig_atomic_t g_initialized_destination_signal_caught = 0;
static sigjmp_buf g_initialized_destination_jump_buffer;

static void initialized_destination_signal_handler(int signal_value)
{
    g_initialized_destination_signal_caught = signal_value;
    siglongjmp(g_initialized_destination_jump_buffer, 1);
    return ;
}

template <typename TypeName>
static int expect_sigabrt_copy_into_initialized_destination()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialized_destination_signal_handler;
    new_action_iot.sa_handler = &initialized_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    g_initialized_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialized_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(source);
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_initialized_destination_signal_caught == SIGABRT)
        return (1);
    return (g_initialized_destination_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_sigabrt_move_into_initialized_destination()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialized_destination_signal_handler;
    new_action_iot.sa_handler = &initialized_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    g_initialized_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialized_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(static_cast<TypeName &&>(source));
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_initialized_destination_signal_caught == SIGABRT)
        return (1);
    return (g_initialized_destination_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialized_destination_signal_handler;
    new_action_iot.sa_handler = &initialized_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.destroy() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    g_initialized_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialized_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(source);
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_initialized_destination_signal_caught == SIGABRT)
        return (1);
    return (g_initialized_destination_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_move_from_initialized_source_into_destroyed_then_initialized_aborts()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    TypeName source;
    TypeName destination;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &initialized_destination_signal_handler;
    new_action_iot.sa_handler = &initialized_destination_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }
    if (source.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (destination.destroy() != FT_ERR_SUCCESS)
        return (0);
    if (destination.initialize() != FT_ERR_SUCCESS)
        return (0);
    g_initialized_destination_signal_caught = 0;
    jump_result = sigsetjmp(g_initialized_destination_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination.initialize(static_cast<TypeName &&>(source));
    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    if (g_initialized_destination_signal_caught == SIGABRT)
        return (1);
    return (g_initialized_destination_signal_caught == SIGIOT);
}

FT_TEST(test_price_copy_into_initialized_destination_aborts, "price definition copy initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialized_destination<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_move_into_initialized_destination_aborts, "price definition move initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialized_destination<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_copy_into_reinitialized_destination_aborts, "price definition copy initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_move_into_reinitialized_destination_aborts, "price definition move initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_move_from_initialized_source_into_destroyed_then_initialized_aborts<ft_price_definition>());
    return (1);
}

FT_TEST(test_rarity_copy_into_initialized_destination_aborts, "rarity band copy initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialized_destination<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_into_initialized_destination_aborts, "rarity band move initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialized_destination<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_copy_into_reinitialized_destination_aborts, "rarity band copy initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_into_reinitialized_destination_aborts, "rarity band move initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_move_from_initialized_source_into_destroyed_then_initialized_aborts<ft_rarity_band>());
    return (1);
}

FT_TEST(test_vendor_copy_into_initialized_destination_aborts, "vendor profile copy initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialized_destination<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_into_initialized_destination_aborts, "vendor profile move initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialized_destination<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_copy_into_reinitialized_destination_aborts, "vendor profile copy initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_into_reinitialized_destination_aborts, "vendor profile move initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_move_from_initialized_source_into_destroyed_then_initialized_aborts<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_currency_copy_into_initialized_destination_aborts, "currency rate copy initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialized_destination<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_into_initialized_destination_aborts, "currency rate move initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialized_destination<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_copy_into_reinitialized_destination_aborts, "currency rate copy initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_into_reinitialized_destination_aborts, "currency rate move initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_move_from_initialized_source_into_destroyed_then_initialized_aborts<ft_currency_rate>());
    return (1);
}

FT_TEST(test_table_copy_into_initialized_destination_aborts, "economy table copy initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_into_initialized_destination<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_move_into_initialized_destination_aborts, "economy table move initialize aborts when destination already initialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_into_initialized_destination<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_copy_into_reinitialized_destination_aborts, "economy table copy initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_copy_from_initialized_source_into_destroyed_then_initialized_aborts<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_move_into_reinitialized_destination_aborts, "economy table move initialize aborts when destination has been reinitialized")
{
    FT_ASSERT_EQ(1, expect_move_from_initialized_source_into_destroyed_then_initialized_aborts<ft_economy_table>());
    return (1);
}
