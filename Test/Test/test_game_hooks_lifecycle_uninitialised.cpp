#include "../../Modules/Game/game_hooks.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void hooks_set_item_callback(game_hooks &value)
{
    ft_function<void(game_character &, game_item &)> callback;

    value.set_on_item_crafted(
        static_cast<ft_function<void(game_character &, game_item &)> &&>(
            callback));
    return ;
}

static void hooks_set_damage_callback(game_hooks &value)
{
    ft_function<void(game_character &, int32_t, uint8_t)> callback;

    value.set_on_character_damaged(
        static_cast<ft_function<void(game_character &, int32_t, uint8_t)> &&>(
            callback));
    return ;
}

static void hooks_set_event_callback(game_hooks &value)
{
    ft_function<void(game_world &, game_event &)> callback;

    value.set_on_event_triggered(
        static_cast<ft_function<void(game_world &, game_event &)> &&>(
            callback));
    return ;
}

static void hooks_get_item_callback(game_hooks &value)
{
    (void)value.get_on_item_crafted();
    return ;
}

static void hooks_get_damage_callback(game_hooks &value)
{
    (void)value.get_on_character_damaged();
    return ;
}

static void hooks_get_event_callback(game_hooks &value)
{
    (void)value.get_on_event_triggered();
    return ;
}

static void hooks_invoke_item_callback(game_hooks &value)
{
    game_character character;
    game_item item;

    value.invoke_on_item_crafted(character, item);
    return ;
}

static void hooks_invoke_damage_callback(game_hooks &value)
{
    game_character character;

    value.invoke_on_character_damaged(character, 1, 0);
    return ;
}

static void hooks_invoke_event_callback(game_hooks &value)
{
    game_world world;
    game_event event;

    value.invoke_on_event_triggered(world, event);
    return ;
}

static void hooks_register_listener(game_hooks &value)
{
    ft_game_hook_metadata metadata;
    ft_function<void(ft_game_hook_context &)> callback;

    value.register_listener(
        metadata, 1,
        static_cast<ft_function<void(ft_game_hook_context &)> &&>(callback));
    return ;
}

static void hooks_unregister_listener(game_hooks &value)
{
    ft_string hook_identifier;
    ft_string listener_name;

    (void)hook_identifier.initialize("hook");
    (void)listener_name.initialize("listener");
    value.unregister_listener(hook_identifier, listener_name);
    return ;
}

static void hooks_get_catalog_metadata(game_hooks &value)
{
    (void)value.get_catalog_metadata();
    return ;
}

static void hooks_get_catalog_metadata_for(game_hooks &value)
{
    ft_string hook_identifier;

    (void)hook_identifier.initialize("hook");
    (void)value.get_catalog_metadata_for(hook_identifier);
    return ;
}

static void hooks_reset(game_hooks &value)
{
    value.reset();
    return ;
}

static void hooks_enable_thread_safety(game_hooks &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void hooks_disable_thread_safety(game_hooks &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void hooks_is_thread_safe(game_hooks &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void hooks_lock(game_hooks &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void hooks_unlock(game_hooks &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void hooks_get_error(game_hooks &value)
{
    (void)value.get_error();
    return ;
}

static void hooks_get_error_str(game_hooks &value)
{
    (void)value.get_error_str();
    return ;
}

static void hooks_destroy(game_hooks &value)
{
    (void)value.destroy();
    return ;
}

static void hooks_initialize_twice(game_hooks &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void hooks_move_uninitialised(game_hooks &value)
{
    game_hooks source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_hooks_set_item_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_set_item_callback));
    return (1);
}

FT_TEST(test_game_hooks_set_damage_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_set_damage_callback));
    return (1);
}

FT_TEST(test_game_hooks_set_event_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_set_event_callback));
    return (1);
}

FT_TEST(test_game_hooks_get_item_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_get_item_callback));
    return (1);
}

FT_TEST(test_game_hooks_get_damage_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_get_damage_callback));
    return (1);
}

FT_TEST(test_game_hooks_get_event_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_get_event_callback));
    return (1);
}

FT_TEST(test_game_hooks_invoke_item_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_invoke_item_callback));
    return (1);
}

FT_TEST(test_game_hooks_invoke_damage_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_invoke_damage_callback));
    return (1);
}

FT_TEST(test_game_hooks_invoke_event_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_invoke_event_callback));
    return (1);
}

FT_TEST(test_game_hooks_register_listener_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_register_listener));
    return (1);
}

FT_TEST(test_game_hooks_unregister_listener_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_unregister_listener));
    return (1);
}

FT_TEST(test_game_hooks_get_catalog_metadata_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_get_catalog_metadata));
    return (1);
}

FT_TEST(test_game_hooks_get_catalog_metadata_for_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_get_catalog_metadata_for));
    return (1);
}

FT_TEST(test_game_hooks_reset_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_reset));
    return (1);
}

FT_TEST(test_game_hooks_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_hooks_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_hooks>(
                        hooks_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_hooks_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_hooks>(hooks_is_thread_safe));
    return (1);
}

FT_TEST(test_game_hooks_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_lock));
    return (1);
}

FT_TEST(test_game_hooks_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_unlock));
    return (1);
}

FT_TEST(test_game_hooks_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_get_error));
    return (1);
}

FT_TEST(test_game_hooks_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_get_error_str));
    return (1);
}

FT_TEST(test_game_hooks_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_hooks>(hooks_destroy));
    return (1);
}

FT_TEST(test_game_hooks_initialize_twice_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_initialize_twice));
    return (1);
}

FT_TEST(test_game_hooks_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_hooks>(hooks_move_uninitialised));
    return (1);
}
