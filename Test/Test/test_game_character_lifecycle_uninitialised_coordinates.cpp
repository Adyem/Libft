#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_get_x(game_character &value)
{
    (void)value.get_x();
    return ;
}

static void game_character_set_x(game_character &value)
{
    value.set_x(1);
    return ;
}

static void game_character_get_y(game_character &value)
{
    (void)value.get_y();
    return ;
}

static void game_character_set_y(game_character &value)
{
    value.set_y(1);
    return ;
}

static void game_character_get_z(game_character &value)
{
    (void)value.get_z();
    return ;
}

static void game_character_set_z(game_character &value)
{
    value.set_z(1);
    return ;
}

static void game_character_move(game_character &value)
{
    value.move(1, 1, 1);
    return ;
}

FT_TEST(test_game_character_get_x_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_get_x));
    return (1);
}

FT_TEST(test_game_character_set_x_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_set_x));
    return (1);
}

FT_TEST(test_game_character_get_y_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_get_y));
    return (1);
}

FT_TEST(test_game_character_set_y_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_set_y));
    return (1);
}

FT_TEST(test_game_character_get_z_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_get_z));
    return (1);
}

FT_TEST(test_game_character_set_z_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_set_z));
    return (1);
}

FT_TEST(test_game_character_move_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_character>(game_character_move));
    return (1);
}
