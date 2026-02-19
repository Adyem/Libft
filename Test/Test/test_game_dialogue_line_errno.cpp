#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_constructs_with_parameters, "parameterized constructor populates dialogue fields")
{
    ft_vector<int> next_lines;
    next_lines.push_back(5);
    next_lines.push_back(8);

    ft_dialogue_line line(3, ft_string("npc"), ft_string("hello"), next_lines);
    FT_ASSERT_EQ(3, line.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("hello"), line.get_text());
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

    next_lines.push_back(2);
    next_lines.push_back(4);
    line.set_line_id(7);
    line.set_speaker(ft_string("guide"));
    line.set_text(ft_string("update"));
    line.set_next_line_ids(next_lines);

    FT_ASSERT_EQ(7, line.get_line_id());
    FT_ASSERT_EQ(ft_string("guide"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("update"), line.get_text());
    FT_ASSERT_EQ(2u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_conditional_vector_access, "next line getter returns current data")
{
    ft_vector<int> ids;
    ids.push_back(9);
    ids.push_back(11);
    ft_dialogue_line line(4, ft_string("ally"), ft_string("hint"), ids);

    const ft_vector<int> &snapshot = line.get_next_line_ids();
    FT_ASSERT_EQ(2u, snapshot.size());
    FT_ASSERT_EQ(9, snapshot[0]);
    FT_ASSERT_EQ(11, snapshot[1]);

    ft_vector<int> &editable = line.get_next_line_ids();
    editable.push_back(13);
    FT_ASSERT_EQ(3u, line.get_next_line_ids().size());
    return (1);
}
