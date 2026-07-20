#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void dialogue_destroyed_set_text(game_dialogue_line &value)
{
    ft_string text;

    (void)text.initialize();
    value.set_text(text);
    return ;
}

static void dialogue_destroyed_get_next_ids_const(game_dialogue_line &value)
{
    const game_dialogue_line &constant_value = value;

    (void)constant_value.get_next_line_ids();
    return ;
}

static void dialogue_destroyed_get_next_ids_mutable(game_dialogue_line &value)
{
    (void)value.get_next_line_ids();
    return ;
}

static void dialogue_destroyed_set_next_ids(game_dialogue_line &value)
{
    ft_vector<int32_t> next_ids;

    (void)next_ids.initialize();
    value.set_next_line_ids(next_ids);
    return ;
}

static void dialogue_destroyed_is_thread_safe(game_dialogue_line &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_dialogue_line_destroyed_set_text_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_set_text));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_next_ids_const_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_next_ids_const));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_next_ids_mutable_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_get_next_ids_mutable));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_set_next_ids_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_set_next_ids));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_disable_thread_safety_is_safe)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_dialogue_line>(
                        dialogue_destroyed_is_thread_safe));
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_get_error_str_is_valid)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_is_initialised_is_false)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_initialised());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_state_can_reinitialize)
{
    game_dialogue_line value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_TRUE, value.is_initialised());
    return (1);
}

FT_TEST(test_game_dialogue_line_destroyed_destructor_after_cleanup_is_safe)
{
    game_dialogue_line *value;

    value = new game_dialogue_line();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
