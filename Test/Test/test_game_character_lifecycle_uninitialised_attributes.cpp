#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_character_get_endurance(game_character &value)
{
    (void)value.get_endurance();
    return ;
}

static void game_character_set_endurance(game_character &value)
{
    value.set_endurance(1);
    return ;
}

static void game_character_get_reason(game_character &value)
{
    (void)value.get_reason();
    return ;
}

static void game_character_set_reason(game_character &value)
{
    value.set_reason(1);
    return ;
}

static void game_character_get_insigh(game_character &value)
{
    (void)value.get_insigh();
    return ;
}

static void game_character_set_insigh(game_character &value)
{
    value.set_insigh(1);
    return ;
}

static void game_character_get_presence(game_character &value)
{
    (void)value.get_presence();
    return ;
}

static void game_character_set_presence(game_character &value)
{
    value.set_presence(1);
    return ;
}

static void game_character_get_coins(game_character &value)
{
    (void)value.get_coins();
    return ;
}

static void game_character_set_coins(game_character &value)
{
    value.set_coins(1);
    return ;
}

static void game_character_add_coins(game_character &value)
{
    value.add_coins(1);
    return ;
}

static void game_character_sub_coins(game_character &value)
{
    value.sub_coins(1);
    return ;
}

static void game_character_get_valor(game_character &value)
{
    (void)value.get_valor();
    return ;
}

static void game_character_set_valor(game_character &value)
{
    value.set_valor(1);
    return ;
}

static void game_character_add_valor(game_character &value)
{
    value.add_valor(1);
    return ;
}

static void game_character_sub_valor(game_character &value)
{
    value.sub_valor(1);
    return ;
}

static void game_character_get_experience(game_character &value)
{
    (void)value.get_experience();
    return ;
}

static void game_character_set_experience(game_character &value)
{
    value.set_experience(1);
    return ;
}

static void game_character_add_experience(game_character &value)
{
    value.add_experience(1);
    return ;
}

static void game_character_sub_experience(game_character &value)
{
    value.sub_experience(1);
    return ;
}

FT_TEST(test_game_character_get_endurance_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_endurance));
    return (1);
}

FT_TEST(test_game_character_set_endurance_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_endurance));
    return (1);
}

FT_TEST(test_game_character_get_reason_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_reason));
    return (1);
}

FT_TEST(test_game_character_set_reason_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_reason));
    return (1);
}

FT_TEST(test_game_character_get_insigh_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_insigh));
    return (1);
}

FT_TEST(test_game_character_set_insigh_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_insigh));
    return (1);
}

FT_TEST(test_game_character_get_presence_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_presence));
    return (1);
}

FT_TEST(test_game_character_set_presence_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_presence));
    return (1);
}

FT_TEST(test_game_character_get_coins_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_coins));
    return (1);
}

FT_TEST(test_game_character_set_coins_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_coins));
    return (1);
}

FT_TEST(test_game_character_add_coins_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_add_coins));
    return (1);
}

FT_TEST(test_game_character_sub_coins_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_sub_coins));
    return (1);
}

FT_TEST(test_game_character_get_valor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_valor));
    return (1);
}

FT_TEST(test_game_character_set_valor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_valor));
    return (1);
}

FT_TEST(test_game_character_add_valor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_add_valor));
    return (1);
}

FT_TEST(test_game_character_sub_valor_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_sub_valor));
    return (1);
}

FT_TEST(test_game_character_get_experience_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_get_experience));
    return (1);
}

FT_TEST(test_game_character_set_experience_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_set_experience));
    return (1);
}

FT_TEST(test_game_character_add_experience_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_add_experience));
    return (1);
}

FT_TEST(test_game_character_sub_experience_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_character>(
                        game_character_sub_experience));
    return (1);
}
