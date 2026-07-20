#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/Game/game_hooks.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void state_destroyed_set_hooks(game_state &value)
{
    ft_sharedptr<game_hooks> hooks;

    value.set_hooks(hooks);
    return ;
}

static void state_destroyed_get_hooks(game_state &value)
{
    (void)value.get_hooks();
    return ;
}

static void state_destroyed_dispatch_item(game_state &value)
{
    game_character character;
    game_item item;

    value.dispatch_item_crafted(character, item);
    return ;
}

static void state_destroyed_dispatch_damage(game_state &value)
{
    game_character character;

    value.dispatch_character_damaged(character, 1, FT_DAMAGE_PHYSICAL);
    return ;
}

static void state_destroyed_dispatch_event(game_state &value)
{
    game_world world;
    game_event event;

    value.dispatch_event_triggered(world, event);
    return ;
}

static void state_destroyed_disable_thread_safety(game_state &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void state_destroyed_is_thread_safe(game_state &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_state_destroyed_set_hooks_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_set_hooks));
    return (1);
}

FT_TEST(test_game_state_destroyed_get_hooks_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_get_hooks));
    return (1);
}

FT_TEST(test_game_state_destroyed_dispatch_item_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_dispatch_item));
    return (1);
}

FT_TEST(test_game_state_destroyed_dispatch_damage_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_dispatch_damage));
    return (1);
}

FT_TEST(test_game_state_destroyed_dispatch_event_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_dispatch_event));
    return (1);
}

FT_TEST(test_game_state_destroyed_get_error_str_is_valid)
{
    game_state value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_state_destroyed_disable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_state_destroyed_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_is_thread_safe));
    return (1);
}

FT_TEST(test_game_state_destroyed_state_can_reinitialize_again)
{
    game_state value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(1, value.get_worlds().size());
    return (1);
}

FT_TEST(test_game_state_destroyed_destructor_after_cleanup_is_safe)
{
    game_state *value;

    value = new game_state();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
