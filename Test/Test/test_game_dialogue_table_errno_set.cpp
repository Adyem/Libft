#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<ft_dialogue_line> make_line(int id, const char *speaker,
        const char *text)
{
    ft_vector<int> next;
    next.push_back(id + 1);
    ft_sharedptr<ft_dialogue_line> stored(new (std::nothrow) ft_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    ft_string speaker_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker_string.initialize(speaker));
    ft_string text_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text_string.initialize(text));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id, speaker_string,
        text_string, next));
    return (stored);
}

FT_TEST(test_dialogue_table_register_line_success, "register_line stores a new line and allows fetching")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_dialogue_line stored;
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored.initialize(1, npc,
        hello, ft_vector<int>()));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(stored));
    ft_dialogue_line fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched));
    FT_ASSERT_EQ(1, fetched.get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_register_script_success, "register_script stores metadata and lines")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    ft_string quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize("quest"));
    ft_string start;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, start.initialize("start"));
    script.set_title(quest);
    script.set_summary(start);
    script.set_start_line_id(2);
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    lines.push_back(make_line(2, "npc", "reply"));
    script.set_lines(lines);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    ft_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    FT_ASSERT_EQ(5, fetched.get_script_id());
    FT_ASSERT_STR_EQ("quest", fetched.get_title().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_lines()[0]->get_error());
    return (1);
}
