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

static volatile sig_atomic_t g_source_state_signal_caught = 0;
static sigjmp_buf g_source_state_jump_buffer;

static void source_state_signal_handler(int signal_value)
{
    g_source_state_signal_caught = signal_value;
    siglongjmp(g_source_state_jump_buffer, 1);
    return ;
}

static int source_state_install_signal_handlers(
    struct sigaction &old_action_abort,
    struct sigaction &old_action_iot,
    bool &iot_handler_installed)
{
    struct sigaction new_action_abort;
    struct sigaction new_action_iot;

    std::memset(&old_action_abort, 0, sizeof(old_action_abort));
    std::memset(&old_action_iot, 0, sizeof(old_action_iot));
    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &source_state_signal_handler;
    new_action_iot.sa_handler = &source_state_signal_handler;
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
    return (1);
}

static int source_state_restore_signal_handlers(
    const struct sigaction &old_action_abort,
    const struct sigaction &old_action_iot,
    bool iot_handler_installed)
{
    if (sigaction(SIGABRT, &old_action_abort, ft_nullptr) != 0)
        return (0);
    if (iot_handler_installed == true)
    {
        if (sigaction(SIGIOT, &old_action_iot, ft_nullptr) != 0)
            return (0);
    }
    return (1);
}

template <typename TypeName>
static int expect_sigabrt_copy_from_uninitialized_source()
{
    struct sigaction old_action_abort;
    struct sigaction old_action_iot;
    bool iot_handler_installed;
    int jump_result;
    TypeName *source_pointer;
    TypeName *destination_pointer;

    if (source_state_install_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
        return (0);
    source_pointer = new (std::nothrow) TypeName();
    destination_pointer = new (std::nothrow) TypeName();
    if (source_pointer == ft_nullptr || destination_pointer == ft_nullptr)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    g_source_state_signal_caught = 0;
    jump_result = sigsetjmp(g_source_state_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination_pointer->initialize(*source_pointer);
    if (source_state_restore_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
    {
        delete source_pointer;
        delete destination_pointer;
        return (0);
    }
    delete source_pointer;
    delete destination_pointer;
    if (g_source_state_signal_caught == SIGABRT)
        return (1);
    return (g_source_state_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_sigabrt_move_from_uninitialized_source()
{
    struct sigaction old_action_abort;
    struct sigaction old_action_iot;
    bool iot_handler_installed;
    int jump_result;
    TypeName *source_pointer;
    TypeName *destination_pointer;

    if (source_state_install_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
        return (0);
    source_pointer = new (std::nothrow) TypeName();
    destination_pointer = new (std::nothrow) TypeName();
    if (source_pointer == ft_nullptr || destination_pointer == ft_nullptr)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    g_source_state_signal_caught = 0;
    jump_result = sigsetjmp(g_source_state_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination_pointer->initialize(
            static_cast<TypeName &&>(*source_pointer));
    if (source_state_restore_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
    {
        delete source_pointer;
        delete destination_pointer;
        return (0);
    }
    delete source_pointer;
    delete destination_pointer;
    if (g_source_state_signal_caught == SIGABRT)
        return (1);
    return (g_source_state_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_sigabrt_copy_from_destroyed_source()
{
    struct sigaction old_action_abort;
    struct sigaction old_action_iot;
    bool iot_handler_installed;
    TypeName *source_pointer;
    TypeName *destination_pointer;
    int jump_result;

    if (source_state_install_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
        return (0);
    source_pointer = new (std::nothrow) TypeName();
    destination_pointer = new (std::nothrow) TypeName();
    if (source_pointer == ft_nullptr || destination_pointer == ft_nullptr)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    if (source_pointer->initialize() != FT_ERR_SUCCESS)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    if (source_pointer->destroy() != FT_ERR_SUCCESS)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    g_source_state_signal_caught = 0;
    jump_result = sigsetjmp(g_source_state_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination_pointer->initialize(*source_pointer);
    if (source_state_restore_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
    {
        delete source_pointer;
        delete destination_pointer;
        return (0);
    }
    delete source_pointer;
    delete destination_pointer;
    if (g_source_state_signal_caught == SIGABRT)
        return (1);
    return (g_source_state_signal_caught == SIGIOT);
}

template <typename TypeName>
static int expect_sigabrt_move_from_destroyed_source()
{
    struct sigaction old_action_abort;
    struct sigaction old_action_iot;
    bool iot_handler_installed;
    TypeName *source_pointer;
    TypeName *destination_pointer;
    int jump_result;

    if (source_state_install_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
        return (0);
    source_pointer = new (std::nothrow) TypeName();
    destination_pointer = new (std::nothrow) TypeName();
    if (source_pointer == ft_nullptr || destination_pointer == ft_nullptr)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    if (source_pointer->initialize() != FT_ERR_SUCCESS)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    if (source_pointer->destroy() != FT_ERR_SUCCESS)
    {
        delete source_pointer;
        delete destination_pointer;
        if (source_state_restore_signal_handlers(old_action_abort,
                old_action_iot, iot_handler_installed) == 0)
            return (0);
        return (0);
    }
    g_source_state_signal_caught = 0;
    jump_result = sigsetjmp(g_source_state_jump_buffer, 1);
    if (jump_result == 0)
        (void)destination_pointer->initialize(
            static_cast<TypeName &&>(*source_pointer));
    if (source_state_restore_signal_handlers(old_action_abort, old_action_iot,
            iot_handler_installed) == 0)
    {
        delete source_pointer;
        delete destination_pointer;
        return (0);
    }
    delete source_pointer;
    delete destination_pointer;
    if (g_source_state_signal_caught == SIGABRT)
        return (1);
    return (g_source_state_signal_caught == SIGIOT);
}

FT_TEST(test_price_copy_from_uninitialized_source_aborts, "price definition copy initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_uninitialized_source<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_move_from_uninitialized_source_aborts, "price definition move initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_uninitialized_source<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_copy_from_destroyed_source_aborts, "price definition copy initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_destroyed_source<ft_price_definition>());
    return (1);
}

FT_TEST(test_price_move_from_destroyed_source_aborts, "price definition move initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_destroyed_source<ft_price_definition>());
    return (1);
}

FT_TEST(test_rarity_copy_from_uninitialized_source_aborts, "rarity band copy initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_uninitialized_source<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_from_uninitialized_source_aborts, "rarity band move initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_uninitialized_source<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_copy_from_destroyed_source_aborts, "rarity band copy initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_destroyed_source<ft_rarity_band>());
    return (1);
}

FT_TEST(test_rarity_move_from_destroyed_source_aborts, "rarity band move initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_destroyed_source<ft_rarity_band>());
    return (1);
}

FT_TEST(test_vendor_copy_from_uninitialized_source_aborts, "vendor profile copy initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_uninitialized_source<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_from_uninitialized_source_aborts, "vendor profile move initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_uninitialized_source<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_copy_from_destroyed_source_aborts, "vendor profile copy initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_destroyed_source<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_vendor_move_from_destroyed_source_aborts, "vendor profile move initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_destroyed_source<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_currency_copy_from_uninitialized_source_aborts, "currency rate copy initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_uninitialized_source<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_from_uninitialized_source_aborts, "currency rate move initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_uninitialized_source<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_copy_from_destroyed_source_aborts, "currency rate copy initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_destroyed_source<ft_currency_rate>());
    return (1);
}

FT_TEST(test_currency_move_from_destroyed_source_aborts, "currency rate move initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_destroyed_source<ft_currency_rate>());
    return (1);
}

FT_TEST(test_table_copy_from_uninitialized_source_aborts, "economy table copy initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_uninitialized_source<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_move_from_uninitialized_source_aborts, "economy table move initialize aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_uninitialized_source<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_copy_from_destroyed_source_aborts, "economy table copy initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_copy_from_destroyed_source<ft_economy_table>());
    return (1);
}

FT_TEST(test_table_move_from_destroyed_source_aborts, "economy table move initialize aborts for destroyed source")
{
    FT_ASSERT_EQ(1, expect_sigabrt_move_from_destroyed_source<ft_economy_table>());
    return (1);
}
