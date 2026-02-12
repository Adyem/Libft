#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_setters, "dialogue line setters replace fields")
{
    ft_dialogue_line line;
    ft_vector<int> next_ids;

    line = ft_dialogue_line(1, ft_string("npc"), ft_string("hi"), next_ids);
    line.set_line_id(9);
    line.set_speaker(ft_string("narrator"));
    line.set_text(ft_string("welcome"));
    next_ids.push_back(10);
    next_ids.push_back(11);
    line.set_next_line_ids(next_ids);

    FT_ASSERT_EQ(9, line.get_line_id());
    FT_ASSERT_EQ(ft_string("narrator"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("welcome"), line.get_text());
    FT_ASSERT_EQ(2, line.get_next_line_ids().size());
    FT_ASSERT_EQ(10, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(11, line.get_next_line_ids()[1]);
    return (1);
}

FT_TEST(test_dialogue_script_setters, "dialogue script setters replace fields and lines")
{
    ft_dialogue_script script;
    ft_vector<int> followups;
    ft_vector<ft_dialogue_line> lines;

    followups.push_back(2);
    lines.push_back(ft_dialogue_line(1, ft_string("npc"), ft_string("hello"), followups));
    script = ft_dialogue_script(3, ft_string("old"), ft_string("first"), 1, lines);

    script.set_script_id(7);
    script.set_title(ft_string("new"));
    script.set_summary(ft_string("updated"));
    script.set_start_line_id(2);
    followups.clear();
    followups.push_back(4);
    lines.clear();
    lines.push_back(ft_dialogue_line(2, ft_string("npc"), ft_string("next"), followups));
    script.set_lines(lines);

    FT_ASSERT_EQ(7, script.get_script_id());
    FT_ASSERT_EQ(ft_string("new"), script.get_title());
    FT_ASSERT_EQ(ft_string("updated"), script.get_summary());
    FT_ASSERT_EQ(2, script.get_start_line_id());
    FT_ASSERT_EQ(1, script.get_lines().size());
    FT_ASSERT_EQ(ft_string("next"), script.get_lines()[0].get_text());
    FT_ASSERT_EQ(4, script.get_lines()[0].get_next_line_ids()[0]);
    return (1);
}
