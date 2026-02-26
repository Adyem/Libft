#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_setters_populate_fields, "setters populate dialogue fields")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line;

    ft_string empty_speaker;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_speaker.initialize(""));
    ft_string empty_text;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_text.initialize(""));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(0, empty_speaker, empty_text, next_lines));

    next_lines.push_back(5);
    next_lines.push_back(8);
    line.set_line_id(3);
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    line.set_speaker(npc);
    line.set_text(hello);
    line.set_next_line_ids(next_lines);
    FT_ASSERT_EQ(3, line.get_line_id());
    FT_ASSERT_STR_EQ("npc", line.get_speaker().c_str());
    FT_ASSERT_STR_EQ("hello", line.get_text().c_str());
    FT_ASSERT_EQ(2u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(5, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(8, line.get_next_line_ids()[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_setters_propagate_success, "setters maintain success state and mutate values")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line;

    ft_string empty_speaker2;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_speaker2.initialize(""));
    ft_string empty_text2;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_text2.initialize(""));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(0, empty_speaker2, empty_text2, next_lines));

    next_lines.push_back(2);
    next_lines.push_back(4);
    line.set_line_id(7);
    ft_string guide;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, guide.initialize("guide"));
    ft_string update;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, update.initialize("update"));
    line.set_speaker(guide);
    line.set_text(update);
    line.set_next_line_ids(next_lines);

    FT_ASSERT_EQ(7, line.get_line_id());
    FT_ASSERT_STR_EQ("guide", line.get_speaker().c_str());
    FT_ASSERT_STR_EQ("update", line.get_text().c_str());
    FT_ASSERT_EQ(2u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_conditional_vector_access, "next line getter returns current data")
{
    ft_vector<int> ids;
    ids.push_back(9);
    ids.push_back(11);
    ft_dialogue_line line;

    ft_string empty_speaker3;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_speaker3.initialize(""));
    ft_string empty_text3;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_text3.initialize(""));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(0, empty_speaker3, empty_text3, ids));

    line.set_line_id(4);
    ft_string ally;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ally.initialize("ally"));
    ft_string hint;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hint.initialize("hint"));
    line.set_speaker(ally);
    line.set_text(hint);
    line.set_next_line_ids(ids);

    const ft_vector<int> &snapshot = line.get_next_line_ids();
    FT_ASSERT_EQ(2u, snapshot.size());
    FT_ASSERT_EQ(9, snapshot[0]);
    FT_ASSERT_EQ(11, snapshot[1]);

    ft_vector<int> &editable = line.get_next_line_ids();
    editable.push_back(13);
    FT_ASSERT_EQ(3u, line.get_next_line_ids().size());
    return (1);
}
