#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_basic_fields, "ft_dialogue_line stores id, speaker, text, and followups")
{
    ft_vector<int> followups;
    followups.push_back(2);
    followups.push_back(4);
    ft_dialogue_line line(1, ft_string("npc"), ft_string("hello"), followups);

    FT_ASSERT_EQ(1, line.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("hello"), line.get_text());
    FT_ASSERT_EQ(2u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(2, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(4, line.get_next_line_ids()[1]);
    return (1);
}

FT_TEST(test_dialogue_script_sets_metadata_and_lines, "ft_dialogue_script stores title, start line, and line pointers")
{
    ft_vector<int> followups;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    followups.push_back(5);
    ft_sharedptr<ft_dialogue_line> line(new (std::nothrow) ft_dialogue_line(
                3, ft_string("npc"), ft_string("branch"), followups));
    FT_ASSERT(line.get() != ft_nullptr);

    lines.push_back(line);
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(7);
    script.set_title(ft_string("quest"));
    script.set_summary(ft_string("start"));
    script.set_start_line_id(3);
    script.set_lines(lines);

    FT_ASSERT_EQ(7, script.get_script_id());
    FT_ASSERT_EQ(ft_string("quest"), script.get_title());
    FT_ASSERT_EQ(1u, script.get_lines().size());
    FT_ASSERT_EQ(3, script.get_lines()[0]->get_line_id());
    return (1);
}
