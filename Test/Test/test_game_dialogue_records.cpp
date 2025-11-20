#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../System_utils/test_runner.hpp"

static void build_basic_lines(ft_vector<ft_dialogue_line> &lines)
{
    ft_vector<int> followups;

    followups.push_back(2);
    lines.push_back(ft_dialogue_line());
    lines[0] = ft_dialogue_line(1, ft_string("npc"), ft_string("hello"), followups);
    followups.clear();
    followups.push_back(3);
    lines.push_back(ft_dialogue_line());
    lines[1] = ft_dialogue_line(2, ft_string("player"), ft_string("response"), followups);
    followups.clear();
    lines.push_back(ft_dialogue_line(3, ft_string("npc"), ft_string("bye"), followups));
    return ;
}

FT_TEST(test_dialogue_line_copy_move, "copy and move dialogue lines")
{
    ft_vector<int> next_ids;
    ft_dialogue_line original;
    ft_dialogue_line copy;
    ft_dialogue_line assigned;
    ft_dialogue_line moved;
    ft_dialogue_line moved_assigned;

    next_ids.push_back(4);
    next_ids.push_back(5);
    original = ft_dialogue_line(2, ft_string("guide"), ft_string("welcome"), next_ids);

    copy = ft_dialogue_line(original);
    FT_ASSERT_EQ(2, copy.get_line_id());
    FT_ASSERT_EQ(ft_string("guide"), copy.get_speaker());
    FT_ASSERT_EQ(4, copy.get_next_line_ids()[0]);
    FT_ASSERT_EQ(ft_string("welcome"), copy.get_text());

    assigned = ft_dialogue_line();
    assigned = original;
    FT_ASSERT_EQ(ft_string("welcome"), assigned.get_text());
    FT_ASSERT_EQ(5, assigned.get_next_line_ids()[1]);

    moved = ft_dialogue_line(ft_move(original));
    FT_ASSERT_EQ(ft_string("guide"), moved.get_speaker());
    FT_ASSERT(original.get_speaker().empty());
    FT_ASSERT(original.get_next_line_ids().empty());

    moved_assigned = ft_dialogue_line();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(ft_string("welcome"), moved_assigned.get_text());
    FT_ASSERT(moved.get_speaker().empty());
    FT_ASSERT(moved.get_next_line_ids().empty());
    return (1);
}

FT_TEST(test_dialogue_script_copy_move, "copy and move dialogue scripts")
{
    ft_dialogue_script script;
    ft_dialogue_script copied;
    ft_dialogue_script assigned;
    ft_dialogue_script moved;
    ft_dialogue_script moved_assigned;
    ft_vector<ft_dialogue_line> lines;

    build_basic_lines(lines);
    script = ft_dialogue_script(7, ft_string("intro"), ft_string("start"), 1, lines);

    copied = ft_dialogue_script(script);
    FT_ASSERT_EQ(7, copied.get_script_id());
    FT_ASSERT_EQ(ft_string("intro"), copied.get_title());
    FT_ASSERT_EQ(3, copied.get_lines().size());

    assigned = ft_dialogue_script();
    assigned = script;
    FT_ASSERT_EQ(ft_string("start"), assigned.get_summary());
    FT_ASSERT_EQ(1, assigned.get_start_line_id());

    moved = ft_dialogue_script(ft_move(script));
    FT_ASSERT_EQ(3, moved.get_lines().size());
    FT_ASSERT(script.get_lines().empty());
    FT_ASSERT_EQ(ER_SUCCESS, script.get_error());

    moved_assigned = ft_dialogue_script();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(ft_string("intro"), moved_assigned.get_title());
    FT_ASSERT(moved.get_lines().empty());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}
