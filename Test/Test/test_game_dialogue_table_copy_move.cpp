#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/game_dialogue_line.hpp"
#include "../../Game/game_dialogue_script.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<game_dialogue_line> make_line(int id, const char *speaker,
        const char *text)
{
    ft_vector<int> next;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.get_error());
    next.push_back(id + 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.get_error());
    ft_sharedptr<game_dialogue_line> stored(new (std::nothrow) game_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    ft_string speaker_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, speaker_string.initialize(speaker));
    ft_string text_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text_string.initialize(text));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id, speaker_string,
        text_string, next));
    return (stored);
}

FT_TEST(test_dialogue_table_register_line_retrieves_data)
{
    game_dialogue_table table;
    game_dialogue_line loaded;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_vector<int> next;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.get_error());
    next.push_back(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, next.get_error());
    game_dialogue_line original;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    ft_string npc;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, npc.initialize("npc"));
    ft_string hello;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hello.initialize("hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(1, npc,
        hello, next));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, loaded));
    FT_ASSERT_EQ(1, loaded.get_line_id());
    FT_ASSERT_STR_EQ("hello", loaded.get_text().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_scripts_store_lines_via_sharedptrs)
{
    game_dialogue_table table;
    ft_vector<ft_sharedptr<game_dialogue_line>> lines;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.get_error());
    lines.push_back(make_line(2, "player", "reply"));
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
    script.set_start_line_id(2);
    script.set_lines(lines);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    game_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    const ft_vector<ft_sharedptr<game_dialogue_line>> &fetched_lines = fetched.get_lines();
    FT_ASSERT_EQ(1u, fetched_lines.size());
    FT_ASSERT_EQ(2, fetched_lines[0]->get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_lines[0]->get_error());
    return (1);
}
