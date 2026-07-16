#include "../test_internal.hpp"

#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_dialogue_table_thread_safe_lifecycle)
{
    game_dialogue_table table;
    game_dialogue_line line;
    ft_string speaker;
    ft_string text;
    ft_vector<int32_t> next_line_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker.initialize("npc"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.initialize("stay awhile"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(1, speaker, text, next_line_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(line));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), table.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, line));
    FT_ASSERT_EQ(1, line.get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    return (1);
}
