#include "../../Modules/Game/game_experience_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_experience_table_get_count(game_experience_table &value)
{
    (void)value.get_count();
    return ;
}

static void game_experience_table_get_level(game_experience_table &value)
{
    (void)value.get_level(5);
    return ;
}

static void game_experience_table_get_value(game_experience_table &value)
{
    (void)value.get_value(0);
    return ;
}

static void game_experience_table_set_value(game_experience_table &value)
{
    value.set_value(0, 5);
    return ;
}

static void game_experience_table_resize(game_experience_table &value)
{
    (void)value.resize(4);
    return ;
}

static void
game_experience_table_enable_thread_safety(game_experience_table &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_experience_table_check_for_error(game_experience_table &value)
{
    (void)value.check_for_error();
    return ;
}

static void game_experience_table_get_error(game_experience_table &value)
{
    (void)value.get_error();
    return ;
}

static void game_experience_table_get_error_str(game_experience_table &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_experience_table_destroy(game_experience_table &value)
{
    (void)value.destroy();
    return ;
}

FT_TEST(test_new_game_experience_table_get_count_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_get_count));
    return (1);
}

FT_TEST(test_new_game_experience_table_get_level_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_get_level));
    return (1);
}

FT_TEST(test_new_game_experience_table_get_value_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_get_value));
    return (1);
}

FT_TEST(test_new_game_experience_table_set_value_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_set_value));
    return (1);
}

FT_TEST(test_new_game_experience_table_resize_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_resize));
    return (1);
}

FT_TEST(test_new_game_experience_table_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_enable_thread_safety));
    return (1);
}

FT_TEST(test_new_game_experience_table_check_for_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_check_for_error));
    return (1);
}

FT_TEST(test_new_game_experience_table_get_error_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_get_error));
    return (1);
}

FT_TEST(test_new_game_experience_table_get_error_str_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_get_error_str));
    return (1);
}

FT_TEST(test_new_game_experience_table_destroy_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_experience_table>(
                        game_experience_table_destroy));
    return (1);
}
