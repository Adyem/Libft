#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_dialogue_line_get_line_id(game_dialogue_line &value)
{
    (void)value.get_line_id();
    return ;
}

static void game_dialogue_line_set_line_id(game_dialogue_line &value)
{
    value.set_line_id(4);
    return ;
}

static void game_dialogue_line_enable_thread_safety(game_dialogue_line &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_dialogue_line_get_error(game_dialogue_line &value)
{
    (void)value.get_error();
    return ;
}

static void game_dialogue_line_get_error_str(game_dialogue_line &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_dialogue_line_is_initialised(game_dialogue_line &value)
{
    (void)value.is_initialised();
    return ;
}

static void game_dialogue_line_disable_thread_safety(game_dialogue_line &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_dialogue_line_is_thread_safe(game_dialogue_line &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_dialogue_line_move_self(game_dialogue_line &value)
{
    (void)value.move(value);
    return ;
}

static void game_dialogue_line_destroy(game_dialogue_line &value)
{
    (void)value.destroy();
    return ;
}

FT_TEST(test_new_game_dialogue_line_get_line_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_get_line_id));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_set_line_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_set_line_id));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_enable_thread_safety));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_get_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_get_error));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_get_error_str_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_get_error_str));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_is_initialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_is_initialised));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_disable_thread_safety_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_disable_thread_safety));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_is_thread_safe));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_move_self_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_move_self));
    return (1);
}

FT_TEST(test_new_game_dialogue_line_destroy_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_line>(
                        game_dialogue_line_destroy));
    return (1);
}
