#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Game/ft_behavior_action.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_dialogue_line_property_updates, "Game: dialogue line getters/setters mutate state safely")
{
    ft_vector<int> next_ids;
    ft_dialogue_line line;
    next_ids.push_back(3);
    next_ids.push_back(5);

    line.set_line_id(8);
    line.set_speaker(ft_string("guide"));
    line.set_text(ft_string("step"));
    line.set_next_line_ids(next_ids);

    FT_ASSERT_EQ(8, line.get_line_id());
    FT_ASSERT(line.get_speaker() == ft_string("guide"));
    FT_ASSERT_EQ(2U, line.get_next_line_ids().size());

    next_ids.push_back(11);
    line.set_next_line_ids(next_ids);
    FT_ASSERT_EQ(3U, line.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_game_dialogue_script_line_management, "Game: dialogue script accepts shared pointers without copying the lines")
{
    ft_vector<int> next_ids;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_sharedptr<ft_dialogue_line> first_line(new (std::nothrow) ft_dialogue_line(10,
                ft_string("npc"), ft_string("intro"), next_ids));
    FT_ASSERT(first_line.get() != ft_nullptr);

    lines.push_back(first_line);
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    script.set_title(ft_string("quest"));
    script.set_summary(ft_string("start"));
    script.set_start_line_id(10);
    script.set_lines(lines);

    const ft_vector<ft_sharedptr<ft_dialogue_line>> &script_lines = script.get_lines();
    FT_ASSERT_EQ(1U, script_lines.size());
    FT_ASSERT(script_lines[0].get() != ft_nullptr);
    FT_ASSERT_EQ(10, script_lines[0]->get_line_id());
    return (1);
}

FT_TEST(test_game_behavior_action_move_assignment_resets_source,
        "Game: behavior action move assignment transfers values")
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
