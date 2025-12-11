#include "../../Game/game_dialogue_table.hpp"
#include "../../System_utils/test_runner.hpp"

static int register_entries(ft_dialogue_table &table)
{
    ft_vector<int> followups;
    ft_vector<ft_dialogue_line> lines;
    ft_dialogue_line line;
    ft_dialogue_script script;

    followups.push_back(2);
    line = ft_dialogue_line(1, ft_string("npc"), ft_string("hello"), followups);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.register_line(line));

    followups.clear();
    lines.push_back(line);
    lines.push_back(ft_dialogue_line(2, ft_string("npc"), ft_string("bye"), followups));
    script = ft_dialogue_script(9, ft_string("intro"), ft_string("start"), 1, lines);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.register_script(script));
    return (1);
}

FT_TEST(test_dialogue_table_copy_semantics, "dialogue table copy constructor and assignment")
{
    ft_dialogue_table table;
    ft_dialogue_table copied;
    ft_dialogue_table assigned;
    ft_dialogue_script script;
    ft_dialogue_line line;

    register_entries(table);
    copied = ft_dialogue_table(table);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copied.fetch_script(9, script));
    FT_ASSERT_EQ(ft_string("intro"), script.get_title());
    FT_ASSERT_EQ(2, script.get_lines().size());

    assigned = ft_dialogue_table();
    assigned = table;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, assigned.fetch_line(1, line));
    FT_ASSERT_EQ(ft_string("hello"), line.get_text());
    FT_ASSERT(assigned.get_scripts().find(99) == assigned.get_scripts().end());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(3, line));
    return (1);
}

FT_TEST(test_dialogue_table_move_semantics, "dialogue table move constructor and assignment")
{
    ft_dialogue_table source;
    ft_dialogue_table moved;
    ft_dialogue_table moved_assigned;
    ft_dialogue_script script;

    register_entries(source);
    moved = ft_dialogue_table(ft_move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.fetch_script(9, script));
    FT_ASSERT(source.get_lines().empty());
    FT_ASSERT(source.get_scripts().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());

    moved_assigned = ft_dialogue_table();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved_assigned.fetch_script(9, script));
    FT_ASSERT(moved.get_lines().empty());
    FT_ASSERT(moved.get_scripts().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}
