#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<ft_dialogue_line> create_line(int id,
        const char *speaker, const char *text, const ft_vector<int> &next_ids)
{
    ft_sharedptr<ft_dialogue_line> stored(new (std::nothrow) ft_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    ft_string speaker_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker_string.initialize(speaker));
    ft_string text_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text_string.initialize(text));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id,
        speaker_string, text_string, next_ids));
    return (stored);
}

FT_TEST(test_dialogue_table_register_fetch_lines, "register_line stores lines retrievable by id")
{
    ft_dialogue_table table;
    ft_vector<int> next_ids;
    ft_dialogue_line fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    next_ids.push_back(2);
    next_ids.push_back(3);
    ft_dialogue_line original;
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(1, npc,
        hello, next_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(original));
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

FT_TEST(test_dialogue_table_scripts_copy_lines, "scripts store metadata and independent copies of lines")
{
    ft_dialogue_table table;
    ft_vector<int> next_ids;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    next_ids.push_back(5);
    lines.push_back(create_line(4, "player", "reply", next_ids));

    ft_dialogue_script script;
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

    ft_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    const ft_vector<ft_sharedptr<ft_dialogue_line>> &fetched_lines = fetched.get_lines();
    FT_ASSERT_EQ(1u, fetched_lines.size());
    FT_ASSERT_EQ(4, fetched_lines[0]->get_line_id());
    FT_ASSERT_STR_EQ("reply", fetched_lines[0]->get_text().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
