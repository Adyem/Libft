#include "../test_internal.hpp"

#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_dialogue_line_thread_safe_lifecycle)
{
    game_dialogue_line line;
    ft_string speaker;
    ft_string text;
    ft_vector<int32_t> next_line_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker.initialize("villager"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.initialize("hello there"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        line.initialize(3, speaker, text, next_line_ids));
    FT_ASSERT_EQ(3, line.get_line_id());
    FT_ASSERT_EQ(0, ft_strcmp(line.get_speaker().c_str(), "villager"));
    FT_ASSERT_EQ(0, ft_strcmp(line.get_text().c_str(), "hello there"));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), line.get_next_line_ids().size());
    line.set_line_id(8);
    line.set_speaker(text);
    line.set_text(speaker);
    FT_ASSERT_EQ(8, line.get_line_id());
    FT_ASSERT_EQ(0, ft_strcmp(line.get_speaker().c_str(), "hello there"));
    FT_ASSERT_EQ(0, ft_strcmp(line.get_text().c_str(), "villager"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.destroy());
    return (1);
}
