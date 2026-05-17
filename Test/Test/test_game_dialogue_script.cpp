#include "../test_internal.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int32_t make_line(ft_sharedptr<game_dialogue_line> &stored, int id,
        const char *speaker, const char *text)
{
    ft_vector<int> next;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.push_back(id + 1));
    stored = ft_sharedptr<game_dialogue_line>(new (std::nothrow) game_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    ft_string speaker_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker_string.initialize(speaker));
    ft_string text_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text_string.initialize(text));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id, speaker_string,
        text_string, next));
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_dialogue_script_initial_state)
{
    game_dialogue_script script;
    ft_string empty;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty.initialize());
    FT_ASSERT_EQ(0, script.get_script_id());
    FT_ASSERT_EQ(empty.c_str(), script.get_title().c_str());
    FT_ASSERT_EQ(empty.c_str(), script.get_summary().c_str());
    FT_ASSERT_EQ(0, script.get_start_line_id());
    FT_ASSERT_EQ(0u, script.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_setters_modify_metadata_and_lines)
{
    game_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(4);
    ft_string intro;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, intro.initialize("intro"));
    ft_string desc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, desc.initialize("desc"));
    script.set_title(intro);
    script.set_summary(desc);
    script.set_start_line_id(2);
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    ft_sharedptr<game_dialogue_line> line;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, make_line(line, 2, "npc", "reply"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.push_back(line));
    script.set_lines(lines);

    const ft_vector<ft_sharedptr<game_dialogue_line>> &stored = script.get_lines();
    FT_ASSERT_EQ(1u, stored.size());
    FT_ASSERT_EQ(2, stored[0]->get_line_id());
    FT_ASSERT_STR_EQ("reply", stored[0]->get_text().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored[0]->get_error());
    return (1);
}
