#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_table_setters, "dialogue table setter copies replace maps")
{
    ft_dialogue_table table;
    ft_map<int, ft_dialogue_line> lines;
    ft_map<int, ft_dialogue_script> scripts;
    ft_vector<int> followups;
    ft_vector<ft_dialogue_line> script_lines;
    ft_dialogue_line fetched_line;
    ft_dialogue_script fetched_script;

    followups.push_back(3);
    lines.insert(2, ft_dialogue_line(2, ft_string("npc"), ft_string("hi"), followups));
    script_lines.push_back(ft_dialogue_line(5, ft_string("npc"), ft_string("start"), followups));
    scripts.insert(8, ft_dialogue_script(8, ft_string("intro"), ft_string("summary"), 5, script_lines));

    table.set_lines(lines);
    table.set_scripts(scripts);
    lines.clear();
    scripts.clear();

    FT_ASSERT_EQ(1, table.get_lines().size());
    FT_ASSERT_EQ(1, table.get_scripts().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(2, fetched_line));
    FT_ASSERT_EQ(ft_string("hi"), fetched_line.get_text());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(8, fetched_script));
    FT_ASSERT_EQ(ft_string("intro"), fetched_script.get_title());

    table.get_lines().clear();
    table.get_scripts().clear();
    FT_ASSERT_EQ(0, table.get_lines().size());
    FT_ASSERT_EQ(0, table.get_scripts().size());
    table.set_lines(lines);
    table.set_scripts(scripts);
    FT_ASSERT_EQ(0, table.get_lines().size());
    FT_ASSERT_EQ(0, table.get_scripts().size());
    return (1);
}
