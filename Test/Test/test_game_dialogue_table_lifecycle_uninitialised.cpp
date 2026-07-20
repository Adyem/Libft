#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void dialogue_table_initialize_twice(game_dialogue_table &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void dialogue_table_copy_initialize(game_dialogue_table &value)
{
    game_dialogue_table source;

    (void)value.initialize(source);
    return ;
}

static void dialogue_table_move_initialize(game_dialogue_table &value)
{
    game_dialogue_table source;

    (void)value.initialize(static_cast<game_dialogue_table &&>(source));
    return ;
}

static void dialogue_table_move(game_dialogue_table &value)
{
    game_dialogue_table source;

    (void)value.move(source);
    return ;
}

static void dialogue_table_get_lines(game_dialogue_table &value)
{
    (void)value.get_lines();
    return ;
}

static void dialogue_table_get_lines_const(game_dialogue_table &value)
{
    const game_dialogue_table &const_value = value;

    (void)const_value.get_lines();
    return ;
}

static void dialogue_table_set_lines(game_dialogue_table &value)
{
    ft_map<int32_t, ft_sharedptr<game_dialogue_line>> lines;

    (void)lines.initialize();
    value.set_lines(lines);
    return ;
}

static void dialogue_table_get_scripts(game_dialogue_table &value)
{
    (void)value.get_scripts();
    return ;
}

static void dialogue_table_get_scripts_const(game_dialogue_table &value)
{
    const game_dialogue_table &const_value = value;

    (void)const_value.get_scripts();
    return ;
}

static void dialogue_table_set_scripts(game_dialogue_table &value)
{
    ft_map<int32_t, game_dialogue_script> scripts;

    (void)scripts.initialize();
    value.set_scripts(scripts);
    return ;
}

static void dialogue_table_register_line(game_dialogue_table &value)
{
    game_dialogue_line line;

    (void)value.register_line(line);
    return ;
}

static void dialogue_table_register_script(game_dialogue_table &value)
{
    game_dialogue_script script;

    (void)value.register_script(script);
    return ;
}

static void dialogue_table_fetch_line(game_dialogue_table &value)
{
    game_dialogue_line line;

    (void)value.fetch_line(1, line);
    return ;
}

static void dialogue_table_fetch_script(game_dialogue_table &value)
{
    game_dialogue_script script;

    (void)value.fetch_script(1, script);
    return ;
}

static void dialogue_table_enable_thread_safety(game_dialogue_table &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void dialogue_table_lock(game_dialogue_table &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void dialogue_table_unlock(game_dialogue_table &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void dialogue_table_get_error(game_dialogue_table &value)
{
    (void)value.get_error();
    return ;
}

static void dialogue_table_get_error_str(game_dialogue_table &value)
{
    (void)value.get_error_str();
    return ;
}

static void dialogue_table_destroy(game_dialogue_table &value)
{
    (void)value.destroy();
    return ;
}

FT_TEST(test_dialogue_table_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_initialize_twice));
    return (1);
}

FT_TEST(test_dialogue_table_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_copy_initialize));
    return (1);
}

FT_TEST(test_dialogue_table_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_move_initialize));
    return (1);
}

FT_TEST(test_dialogue_table_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_move));
    return (1);
}

FT_TEST(test_dialogue_table_get_lines_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_lines));
    return (1);
}

FT_TEST(test_dialogue_table_get_lines_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_lines_const));
    return (1);
}

FT_TEST(test_dialogue_table_set_lines_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_set_lines));
    return (1);
}

FT_TEST(test_dialogue_table_get_scripts_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_scripts));
    return (1);
}

FT_TEST(test_dialogue_table_get_scripts_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_scripts_const));
    return (1);
}

FT_TEST(test_dialogue_table_set_scripts_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_set_scripts));
    return (1);
}

FT_TEST(test_dialogue_table_register_line_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_register_line));
    return (1);
}

FT_TEST(test_dialogue_table_register_script_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_register_script));
    return (1);
}

FT_TEST(test_dialogue_table_fetch_line_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_fetch_line));
    return (1);
}

FT_TEST(test_dialogue_table_fetch_script_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_fetch_script));
    return (1);
}

FT_TEST(test_dialogue_table_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_enable_thread_safety));
    return (1);
}

FT_TEST(test_dialogue_table_disable_thread_safety_uninitialised_is_safe)
{
    game_dialogue_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}

FT_TEST(test_dialogue_table_is_thread_safe_uninitialised_is_safe)
{
    game_dialogue_table value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_dialogue_table_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_lock));
    return (1);
}

FT_TEST(test_dialogue_table_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_unlock));
    return (1);
}

FT_TEST(test_dialogue_table_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_error));
    return (1);
}

FT_TEST(test_dialogue_table_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_get_error_str));
    return (1);
}

FT_TEST(test_dialogue_table_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_table>(
                        dialogue_table_destroy));
    return (1);
}
