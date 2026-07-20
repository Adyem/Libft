#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void state_destroyed_get_worlds(game_state &value)
{
    (void)value.get_worlds();
    return ;
}

static void state_destroyed_get_characters(game_state &value)
{
    (void)value.get_characters();
    return ;
}

static void state_destroyed_set_variable(game_state &value)
{
    ft_string key;
    ft_string variable_value;

    (void)key.initialize("key");
    (void)variable_value.initialize("value");
    value.set_variable(key, variable_value);
    return ;
}

static void state_destroyed_get_variable(game_state &value)
{
    ft_string key;

    (void)key.initialize("key");
    (void)value.get_variable(key);
    return ;
}

static void state_destroyed_remove_variable(game_state &value)
{
    ft_string key;

    (void)key.initialize("key");
    value.remove_variable(key);
    return ;
}

static void state_destroyed_clear_variables(game_state &value)
{
    value.clear_variables();
    return ;
}

static void state_destroyed_add_character(game_state &value)
{
    ft_sharedptr<game_character> character;

    (void)value.add_character(character);
    return ;
}

static void state_destroyed_remove_character(game_state &value)
{
    value.remove_character(0);
    return ;
}

static void state_destroyed_reset_hooks(game_state &value)
{
    value.reset_hooks();
    return ;
}

static void state_destroyed_enable_thread_safety(game_state &value)
{
    (void)value.enable_thread_safety();
    return ;
}

FT_TEST(test_game_state_destroyed_get_worlds_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_get_worlds));
    return (1);
}

FT_TEST(test_game_state_destroyed_get_characters_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_get_characters));
    return (1);
}

FT_TEST(test_game_state_destroyed_set_variable_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_set_variable));
    return (1);
}

FT_TEST(test_game_state_destroyed_get_variable_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_get_variable));
    return (1);
}

FT_TEST(test_game_state_destroyed_remove_variable_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_remove_variable));
    return (1);
}

FT_TEST(test_game_state_destroyed_clear_variables_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_clear_variables));
    return (1);
}

FT_TEST(test_game_state_destroyed_add_character_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_add_character));
    return (1);
}

FT_TEST(test_game_state_destroyed_remove_character_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_remove_character));
    return (1);
}

FT_TEST(test_game_state_destroyed_reset_hooks_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_reset_hooks));
    return (1);
}

FT_TEST(test_game_state_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_state>(
                        state_destroyed_enable_thread_safety));
    return (1);
}
