#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void dialogue_script_get_id(game_dialogue_script &value)
{
    (void)value.get_script_id();
    return ;
}

static void dialogue_script_set_id(game_dialogue_script &value)
{
    value.set_script_id(1);
    return ;
}

static void dialogue_script_get_title(game_dialogue_script &value)
{
    (void)value.get_title();
    return ;
}

static void dialogue_script_set_title(game_dialogue_script &value)
{
    ft_string title;

    (void)title.initialize("title");
    value.set_title(title);
    return ;
}

static void dialogue_script_get_summary(game_dialogue_script &value)
{
    (void)value.get_summary();
    return ;
}

static void dialogue_script_set_summary(game_dialogue_script &value)
{
    ft_string summary;

    (void)summary.initialize("summary");
    value.set_summary(summary);
    return ;
}

static void dialogue_script_get_start_line_id(game_dialogue_script &value)
{
    (void)value.get_start_line_id();
    return ;
}

static void dialogue_script_set_start_line_id(game_dialogue_script &value)
{
    value.set_start_line_id(2);
    return ;
}

static void dialogue_script_get_lines(game_dialogue_script &value)
{
    (void)value.get_lines();
    return ;
}

static void dialogue_script_get_lines_const(game_dialogue_script &value)
{
    const game_dialogue_script &const_value = value;

    (void)const_value.get_lines();
    return ;
}

static void dialogue_script_set_lines(game_dialogue_script &value)
{
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;

    (void)lines.initialize();
    value.set_lines(lines);
    return ;
}

static void dialogue_script_enable_thread_safety(game_dialogue_script &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void dialogue_script_lock(game_dialogue_script &value)
{
    ft_bool lock_acquired = FT_FALSE;

    (void)value.lock(&lock_acquired);
    return ;
}

static void dialogue_script_unlock(game_dialogue_script &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void dialogue_script_get_error(game_dialogue_script &value)
{
    (void)value.get_error();
    return ;
}

static void dialogue_script_get_error_str(game_dialogue_script &value)
{
    (void)value.get_error_str();
    return ;
}

static void dialogue_script_destroy(game_dialogue_script &value)
{
    (void)value.destroy();
    return ;
}

static void dialogue_script_initialize_twice(game_dialogue_script &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void dialogue_script_copy_initialize(game_dialogue_script &value)
{
    game_dialogue_script source;

    (void)value.initialize(source);
    return ;
}

static void dialogue_script_move_initialize(game_dialogue_script &value)
{
    game_dialogue_script source;

    (void)value.initialize(static_cast<game_dialogue_script &&>(source));
    return ;
}

static void dialogue_script_move(game_dialogue_script &value)
{
    game_dialogue_script source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_dialogue_script_get_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_id));
    return (1);
}

FT_TEST(test_dialogue_script_set_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_set_id));
    return (1);
}

FT_TEST(test_dialogue_script_get_title_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_title));
    return (1);
}

FT_TEST(test_dialogue_script_set_title_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_set_title));
    return (1);
}

FT_TEST(test_dialogue_script_get_summary_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_summary));
    return (1);
}

FT_TEST(test_dialogue_script_set_summary_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_set_summary));
    return (1);
}

FT_TEST(test_dialogue_script_get_start_line_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_start_line_id));
    return (1);
}

FT_TEST(test_dialogue_script_set_start_line_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_set_start_line_id));
    return (1);
}

FT_TEST(test_dialogue_script_get_lines_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_lines));
    return (1);
}

FT_TEST(test_dialogue_script_get_lines_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_lines_const));
    return (1);
}

FT_TEST(test_dialogue_script_set_lines_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_set_lines));
    return (1);
}

FT_TEST(test_dialogue_script_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_enable_thread_safety));
    return (1);
}

FT_TEST(test_dialogue_script_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_lock));
    return (1);
}

FT_TEST(test_dialogue_script_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_unlock));
    return (1);
}

FT_TEST(test_dialogue_script_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_error));
    return (1);
}

FT_TEST(test_dialogue_script_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_get_error_str));
    return (1);
}

FT_TEST(test_dialogue_script_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_destroy));
    return (1);
}

FT_TEST(test_dialogue_script_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_initialize_twice));
    return (1);
}

FT_TEST(test_dialogue_script_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_copy_initialize));
    return (1);
}

FT_TEST(test_dialogue_script_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_move_initialize));
    return (1);
}

FT_TEST(test_dialogue_script_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_dialogue_script>(
                        dialogue_script_move));
    return (1);
}

FT_TEST(test_dialogue_script_disable_thread_safety_uninitialised_is_safe)
{
    game_dialogue_script value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}

FT_TEST(test_dialogue_script_is_thread_safe_uninitialised_is_safe)
{
    game_dialogue_script value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}
