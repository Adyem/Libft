#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/Game/game_hooks.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_state_get_worlds(game_state &value)
{
    (void)value.get_worlds();
    return ;
}

static void game_state_get_characters(game_state &value)
{
    (void)value.get_characters();
    return ;
}

static void game_state_set_variable(game_state &value)
{
    ft_string key;
    ft_string variable_value;

    value.set_variable(key, variable_value);
    return ;
}

static void game_state_get_variable(game_state &value)
{
    ft_string key;

    (void)value.get_variable(key);
    return ;
}

static void game_state_remove_variable(game_state &value)
{
    ft_string key;

    value.remove_variable(key);
    return ;
}

static void game_state_clear_variables(game_state &value)
{
    value.clear_variables();
    return ;
}

static void game_state_add_character(game_state &value)
{
    ft_sharedptr<game_character> character;

    (void)value.add_character(character);
    return ;
}

static void game_state_remove_character(game_state &value)
{
    value.remove_character(0U);
    return ;
}

static void game_state_set_hooks(game_state &value)
{
    ft_sharedptr<game_hooks> hooks;

    value.set_hooks(hooks);
    return ;
}

static void game_state_get_hooks(game_state &value)
{
    (void)value.get_hooks();
    return ;
}

static void game_state_reset_hooks(game_state &value)
{
    value.reset_hooks();
    return ;
}

static void game_state_dispatch_item_crafted(game_state &value)
{
    game_character character;
    game_item item;

    value.dispatch_item_crafted(character, item);
    return ;
}

static void game_state_dispatch_character_damaged(game_state &value)
{
    game_character character;

    value.dispatch_character_damaged(character, 1, 0U);
    return ;
}

static void game_state_dispatch_event_triggered(game_state &value)
{
    game_world world;
    game_event event;

    value.dispatch_event_triggered(world, event);
    return ;
}

static void game_state_get_error(game_state &value)
{
    (void)value.get_error();
    return ;
}

static void game_state_get_error_str(game_state &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_state_enable_thread_safety(game_state &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_state_disable_thread_safety(game_state &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_state_is_thread_safe(game_state &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_state_get_worlds_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_get_worlds));
    return (1);
}

FT_TEST(test_game_state_get_characters_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_get_characters));
    return (1);
}

FT_TEST(test_game_state_set_variable_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_set_variable));
    return (1);
}

FT_TEST(test_game_state_get_variable_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_get_variable));
    return (1);
}

FT_TEST(test_game_state_remove_variable_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_remove_variable));
    return (1);
}

FT_TEST(test_game_state_clear_variables_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_clear_variables));
    return (1);
}

FT_TEST(test_game_state_add_character_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_add_character));
    return (1);
}

FT_TEST(test_game_state_remove_character_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_remove_character));
    return (1);
}

FT_TEST(test_game_state_set_hooks_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_set_hooks));
    return (1);
}

FT_TEST(test_game_state_get_hooks_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_get_hooks));
    return (1);
}

FT_TEST(test_game_state_reset_hooks_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_reset_hooks));
    return (1);
}

FT_TEST(test_game_state_dispatch_item_crafted_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_dispatch_item_crafted));
    return (1);
}

FT_TEST(test_game_state_dispatch_character_damaged_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_dispatch_character_damaged));
    return (1);
}

FT_TEST(test_game_state_dispatch_event_triggered_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_dispatch_event_triggered));
    return (1);
}

FT_TEST(test_game_state_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_get_error));
    return (1);
}

FT_TEST(test_game_state_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_state>(game_state_get_error_str));
    return (1);
}

FT_TEST(test_game_state_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_state_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_state_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_state>(
                        game_state_is_thread_safe));
    return (1);
}
