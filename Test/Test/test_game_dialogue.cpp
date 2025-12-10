#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Game/ft_behavior_action.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_dialogue_line_move_resets_source, "Game: dialogue line move constructor clears source state")
{
    ft_vector<int> original_next_ids;

    original_next_ids.push_back(7);
    original_next_ids.push_back(9);
    ft_dialogue_line original(12, ft_string("npc"), ft_string("hello"), original_next_ids);

    ft_dialogue_line moved(ft_move(original));

    FT_ASSERT_EQ(12, moved.get_line_id());
    FT_ASSERT(moved.get_speaker() == ft_string("npc"));
    FT_ASSERT(moved.get_text() == ft_string("hello"));
    FT_ASSERT_EQ(2U, moved.get_next_line_ids().size());
    FT_ASSERT_EQ(0, original.get_line_id());
    FT_ASSERT(original.get_speaker().empty());
    FT_ASSERT(original.get_text().empty());
    FT_ASSERT_EQ(0U, original.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_game_dialogue_line_assignment_deep_copies_successfully, "Game: dialogue line assignment deep copies next ids")
{
    ft_vector<int> next_ids;

    next_ids.push_back(3);
    next_ids.push_back(5);
    ft_dialogue_line source(4, ft_string("guide"), ft_string("step"), next_ids);
    ft_dialogue_line target;

    target = source;
    source.get_next_line_ids().push_back(11);

    FT_ASSERT_EQ(4, target.get_line_id());
    FT_ASSERT(target.get_speaker() == ft_string("guide"));
    FT_ASSERT_EQ(2U, target.get_next_line_ids().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, target.get_error());
    return (1);
}

FT_TEST(test_game_dialogue_script_copy_preserves_lines, "Game: dialogue script copy keeps metadata and lines")
{
    ft_vector<int> next_ids;
    ft_vector<ft_dialogue_line> lines;

    next_ids.push_back(2);
    ft_dialogue_line line(10, ft_string("npc"), ft_string("intro"), next_ids);
    lines.push_back(line);
    ft_dialogue_script script(5, ft_string("quest"), ft_string("start"), 10, lines);

    ft_dialogue_script copy(script);

    FT_ASSERT_EQ(5, copy.get_script_id());
    FT_ASSERT(copy.get_title() == ft_string("quest"));
    FT_ASSERT_EQ(1U, copy.get_lines().size());
    FT_ASSERT_EQ(10, copy.get_lines()[0].get_line_id());
    return (1);
}

FT_TEST(test_game_dialogue_script_move_assignment_clears_source, "Game: dialogue script move assignment clears moved-from state")
{
    ft_vector<ft_dialogue_line> lines;

    lines.push_back(ft_dialogue_line(2, ft_string("npc"), ft_string("branch"), ft_vector<int>()));
    ft_dialogue_script source(8, ft_string("title"), ft_string("summary"), 2, lines);
    ft_dialogue_script destination;

    destination = ft_move(source);

    FT_ASSERT_EQ(8, destination.get_script_id());
    FT_ASSERT(destination.get_summary() == ft_string("summary"));
    FT_ASSERT_EQ(1U, destination.get_lines().size());
    FT_ASSERT_EQ(0, source.get_script_id());
    FT_ASSERT(source.get_title().empty());
    FT_ASSERT_EQ(0U, source.get_lines().size());
    return (1);
}

FT_TEST(test_game_behavior_action_move_assignment_resets_source, "Game: behavior action move assignment transfers values")
{
    ft_behavior_action source(6, 0.5, 1.25);
    ft_behavior_action destination;

    destination = ft_move(source);

    FT_ASSERT_EQ(6, destination.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.5, destination.get_weight());
    FT_ASSERT_DOUBLE_EQ(1.25, destination.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    return (1);
}
