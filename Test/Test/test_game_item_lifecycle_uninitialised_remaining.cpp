#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_item_get_modifier1(game_item &value)
{
    game_item_modifier modifier;

    (void)value.get_modifier1(modifier);
    return ;
}

static void game_item_set_modifier1(game_item &value)
{
    game_item_modifier modifier;

    value.set_modifier1(modifier);
    return ;
}

static void game_item_get_modifier2(game_item &value)
{
    game_item_modifier modifier;

    (void)value.get_modifier2(modifier);
    return ;
}

static void game_item_set_modifier2(game_item &value)
{
    game_item_modifier modifier;

    value.set_modifier2(modifier);
    return ;
}

static void game_item_get_modifier3(game_item &value)
{
    game_item_modifier modifier;

    (void)value.get_modifier3(modifier);
    return ;
}

static void game_item_set_modifier3(game_item &value)
{
    game_item_modifier modifier;

    value.set_modifier3(modifier);
    return ;
}

static void game_item_get_modifier4(game_item &value)
{
    game_item_modifier modifier;

    (void)value.get_modifier4(modifier);
    return ;
}

static void game_item_set_modifier4(game_item &value)
{
    game_item_modifier modifier;

    value.set_modifier4(modifier);
    return ;
}

static void game_item_enable_thread_safety(game_item &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_item_disable_thread_safety(game_item &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_item_is_thread_safe(game_item &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_item_get_error(game_item &value)
{
    (void)value.get_error();
    return ;
}

static void game_item_get_error_str(game_item &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_item_get_modifier1_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_get_modifier1));
    return (1);
}

FT_TEST(test_game_item_set_modifier1_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_set_modifier1));
    return (1);
}

FT_TEST(test_game_item_get_modifier2_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_get_modifier2));
    return (1);
}

FT_TEST(test_game_item_set_modifier2_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_set_modifier2));
    return (1);
}

FT_TEST(test_game_item_get_modifier3_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_get_modifier3));
    return (1);
}

FT_TEST(test_game_item_set_modifier3_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_set_modifier3));
    return (1);
}

FT_TEST(test_game_item_get_modifier4_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_get_modifier4));
    return (1);
}

FT_TEST(test_game_item_set_modifier4_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_set_modifier4));
    return (1);
}

FT_TEST(test_game_item_enable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_is_thread_safe));
    return (1);
}

FT_TEST(test_game_item_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_item>(game_item_get_error));
    return (1);
}

FT_TEST(test_game_item_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(game_item_get_error_str));
    return (1);
}
