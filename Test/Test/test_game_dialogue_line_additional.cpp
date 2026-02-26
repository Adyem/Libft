#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_next_ids_reset, "set_next_line_ids replaces all stored ids")
{
    ft_vector<int> initial_next_lines;
    initial_next_lines.push_back(2);
    initial_next_lines.push_back(4);
    ft_dialogue_line line;

    ft_string speaker;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker.initialize("speaker"));
    ft_string text;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.initialize("text"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(1, speaker,
        text, initial_next_lines));

    line.set_line_id(1);
    line.set_speaker(speaker);
    line.set_text(text);
    line.set_next_line_ids(initial_next_lines);

    ft_vector<int> replacement_next_lines;
    replacement_next_lines.push_back(9);
    line.set_next_line_ids(replacement_next_lines);

    FT_ASSERT_EQ(1u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(9, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_getters_expose_vector_views, "const and non-const getters expose current next ids")
{
    ft_vector<int> ids;
    ids.push_back(7);
    ids.push_back(11);
    ft_dialogue_line line;

    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hint;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hint.initialize("hint"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(3, npc,
        hint, ids));

    line.set_line_id(3);
    line.set_speaker(npc);
    line.set_text(hint);
    line.set_next_line_ids(ids);

    ft_vector<int> &editable = line.get_next_line_ids();
    editable.push_back(15);

    const ft_vector<int> &readonly = line.get_next_line_ids();
    FT_ASSERT_EQ(3u, readonly.size());
    FT_ASSERT_EQ(15, readonly[2]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_setters_apply_latest_values, "setters override previous values each call")
{
    ft_dialogue_line line;

    ft_string empty_speaker;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_speaker.initialize(""));
    ft_string empty_text;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_text.initialize(""));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(0, empty_speaker, empty_text,
        ft_vector<int>()));

    line.set_line_id(5);
    ft_string old_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, old_value.initialize("old"));
    line.set_speaker(old_value);
    ft_string start;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, start.initialize("start"));
    line.set_text(start);

    ft_string middle;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, middle.initialize("middle"));
    line.set_speaker(middle);
    ft_string next_new;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_new.initialize("new"));
    line.set_speaker(next_new);
    ft_string first;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize("first"));
    line.set_text(first);
    ft_string last;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, last.initialize("last"));
    line.set_text(last);
    line.set_line_id(4);
    line.set_line_id(9);

    FT_ASSERT_STR_EQ("new", line.get_speaker().c_str());
    FT_ASSERT_STR_EQ("last", line.get_text().c_str());
    FT_ASSERT_EQ(9, line.get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}
