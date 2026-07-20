#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void dialogue_destroyed_get_line_id(game_dialogue_line &value)
{
    (void)value.get_line_id();
    return ;
}

static void dialogue_destroyed_set_line_id(game_dialogue_line &value)
{
    value.set_line_id(1);
    return ;
}

static void dialogue_destroyed_get_speaker(game_dialogue_line &value)
{
    (void)value.get_speaker();
    return ;
}

static void dialogue_destroyed_set_speaker(game_dialogue_line &value)
{
    ft_string speaker;

    (void)speaker.initialize();
    value.set_speaker(speaker);
    return ;
}

static void dialogue_destroyed_get_text(game_dialogue_line &value)
{
    (void)value.get_text();
    return ;
}

static void dialogue_destroyed_get_next_line_ids(game_dialogue_line &value)
{
    (void)value.get_next_line_ids();
    return ;
}

static void dialogue_destroyed_enable_thread_safety(game_dialogue_line &value)
{
    (void)value.enable_thread_safety();
    return ;
}

FT_TEST(test_game_dialogue_line_destroyed_get_line_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_line_id));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_set_line_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_set_line_id));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_speaker_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_speaker));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_set_speaker_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_set_speaker));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_text_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_text));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_next_line_ids_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_next_line_ids));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_error_is_valid)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_destroy_is_idempotent)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_destructor_is_non_aborting)
{
    game_dialogue_line *value;

    value = new game_dialogue_line();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
