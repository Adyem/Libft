#include "../test_internal.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int32_t create_line(ft_sharedptr<game_dialogue_line> &stored,
        int id, const char *speaker, const char *text,
        const ft_vector<int> &next_ids)
{
    stored = ft_sharedptr<game_dialogue_line>(new (std::nothrow) game_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    ft_string speaker_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker_string.initialize(speaker));
    ft_string text_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text_string.initialize(text));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id,
        speaker_string, text_string, next_ids));
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_dialogue_table_register_fetch_lines)
{
    game_dialogue_table table;
    ft_vector<int> next_ids;
    game_dialogue_line fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.get_error());
    next_ids.push_back(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.get_error());
    next_ids.push_back(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.get_error());
    game_dialogue_line original;
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(1, npc,
        hello, next_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched));
    FT_ASSERT_EQ(1, fetched.get_line_id());
    FT_ASSERT_STR_EQ("npc", fetched.get_speaker().c_str());
    FT_ASSERT_STR_EQ("hello", fetched.get_text().c_str());
    FT_ASSERT_EQ(2, fetched.get_next_line_ids()[0]);
    FT_ASSERT_EQ(3, fetched.get_next_line_ids()[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_scripts_copy_lines)
{
    game_dialogue_table table;
    ft_vector<int> next_ids;
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());
    next_ids.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next_ids.get_error());
    ft_sharedptr<game_dialogue_line> line;
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        create_line(line, 4, "player", "reply", next_ids));
    lines.push_back(line);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());

    game_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    ft_string intro;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, intro.initialize("intro"));
    ft_string start;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, start.initialize("start"));
    script.set_title(intro);
    script.set_summary(start);
    script.set_start_line_id(4);
    script.set_lines(lines);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));

    game_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    const ft_vector<ft_sharedptr<game_dialogue_line>> &fetched_lines = fetched.get_lines();
    FT_ASSERT_EQ(1u, fetched_lines.size());
    FT_ASSERT_EQ(4, fetched_lines[0]->get_line_id());
    FT_ASSERT_STR_EQ("reply", fetched_lines[0]->get_text().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
