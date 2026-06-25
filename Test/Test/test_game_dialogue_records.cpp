#include "../test_internal.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_line_basic_fields)
{
    ft_vector<int> followups;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    followups.push_back(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    followups.push_back(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    game_dialogue_line line;
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.initialize(1, npc,
        hello, followups));

    FT_ASSERT_EQ(1, line.get_line_id());
    FT_ASSERT_STR_EQ("npc", line.get_speaker().c_str());
    FT_ASSERT_STR_EQ("hello", line.get_text().c_str());
    FT_ASSERT_EQ(2u, line.get_next_line_ids().size());
    FT_ASSERT_EQ(2, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(4, line.get_next_line_ids()[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_sets_metadata_and_lines)
{
    ft_vector<int> followups;
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());
    followups.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, followups.get_error());
    ft_sharedptr<game_dialogue_line> line(new (std::nothrow) game_dialogue_line());
    FT_ASSERT(line.get() != ft_nullptr);
    ft_string npc2;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc2.initialize("npc"));
    ft_string branch;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, branch.initialize("branch"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, line->initialize(3, npc2,
        branch, followups));
    FT_ASSERT(line.get() != ft_nullptr);

    lines.push_back(line);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());
    game_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(7);
    ft_string quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize("quest"));
    ft_string start;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, start.initialize("start"));
    script.set_title(quest);
    script.set_summary(start);
    script.set_start_line_id(3);
    script.set_lines(lines);

    FT_ASSERT_EQ(7, script.get_script_id());
    FT_ASSERT_STR_EQ("quest", script.get_title().c_str());
    FT_ASSERT_EQ(1u, script.get_lines().size());
    FT_ASSERT_EQ(3, script.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_lines()[0]->get_error());
    return (1);
}
