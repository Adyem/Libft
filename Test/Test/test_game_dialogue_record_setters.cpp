#include "../test_internal.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_setters_affect_getters)
{
    ft_vector<int> ids;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ids.get_error());
    ids.push_back(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ids.get_error());
    game_dialogue_line line;

    ft_string empty_speaker;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_speaker.initialize(""));
    ft_string empty_text;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_text.initialize(""));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(0, empty_speaker, empty_text, ids));

    line.set_line_id(1);
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hi;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hi.initialize("hi"));
    line.set_speaker(npc);
    line.set_text(hi);
    line.set_next_line_ids(ids);

    FT_ASSERT_EQ(1, line.get_line_id());
    FT_ASSERT_STR_EQ("npc", line.get_speaker().c_str());
    FT_ASSERT_STR_EQ("hi", line.get_text().c_str());
    FT_ASSERT_EQ(1u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(2, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_setters_store_shared_lines)
{
    ft_vector<int> followups;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    followups.push_back(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    ft_sharedptr<game_dialogue_line> line(new (std::nothrow) game_dialogue_line());
    FT_ASSERT(line.get() != ft_nullptr);
    ft_string player;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, player.initialize("player"));
    ft_string reply;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reply.initialize("reply"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->initialize(2, player,
        reply, followups));
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;
    FT_ASSERT(line.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());
    lines.push_back(line);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());

    game_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(3);
    ft_string dialogue;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dialogue.initialize("dialogue"));
    ft_string branch;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, branch.initialize("branch"));
    script.set_title(dialogue);
    script.set_summary(branch);
    script.set_start_line_id(2);
    script.set_lines(lines);

    const ft_vector<ft_sharedptr<game_dialogue_line>> &current_lines = script.get_lines();
    FT_ASSERT_EQ(1u, current_lines.size());
    FT_ASSERT_EQ(2, current_lines[0]->get_line_id());
    FT_ASSERT_STR_EQ("reply", current_lines[0]->get_text().c_str());
    FT_ASSERT_EQ(1u, current_lines[0]->get_next_line_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, current_lines[0]->get_error());
    return (1);
}
