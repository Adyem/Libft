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

    line.set_line_id(1);
    line.set_speaker(ft_string("speaker"));
    line.set_text(ft_string("text"));
    line.set_next_line_ids(initial_next_lines);

    ft_vector<int> replacement_next_lines;
    replacement_next_lines.push_back(9);
    line.set_next_line_ids(replacement_next_lines);

    FT_ASSERT_EQ(1u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(9, line.get_next_line_ids()[0]);
    return (1);
}

FT_TEST(test_dialogue_line_getters_expose_vector_views, "const and non-const getters expose current next ids")
{
    ft_vector<int> ids;
    ids.push_back(7);
    ids.push_back(11);
    ft_dialogue_line line;

    line.set_line_id(3);
    line.set_speaker(ft_string("npc"));
    line.set_text(ft_string("hint"));
    line.set_next_line_ids(ids);

    ft_vector<int> &editable = line.get_next_line_ids();
    editable.push_back(15);

    const ft_vector<int> &readonly = line.get_next_line_ids();
    FT_ASSERT_EQ(3u, readonly.size());
    FT_ASSERT_EQ(15, readonly[2]);
    return (1);
}

FT_TEST(test_dialogue_line_setters_apply_latest_values, "setters override previous values each call")
{
    ft_dialogue_line line;

    line.set_line_id(5);
    line.set_speaker(ft_string("old"));
    line.set_text(ft_string("start"));

    line.set_speaker(ft_string("middle"));
    line.set_speaker(ft_string("new"));
    line.set_text(ft_string("first"));
    line.set_text(ft_string("last"));
    line.set_line_id(4);
    line.set_line_id(9);

    FT_ASSERT_EQ(ft_string("new"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("last"), line.get_text());
    FT_ASSERT_EQ(9, line.get_line_id());
    return (1);
}
