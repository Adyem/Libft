#include "../test_internal.hpp"
#include "../../CPP_class/class_shared_ptr.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int build_line(int id, const char *speaker, const char *text, const ft_vector<int> &next_ids,
        ft_dialogue_line &out_line)
{
    out_line = ft_dialogue_line(id, ft_string(speaker), ft_string(text), next_ids);
    return (1);
}

static ft_sharedptr<ft_dialogue_line> build_line_shared(int id, const char *speaker,
        const char *text, const ft_vector<int> &next_ids)
{
    return (ft_sharedptr<ft_dialogue_line>(new (std::nothrow)
            ft_dialogue_line(id, ft_string(speaker), ft_string(text), next_ids)));
}

FT_TEST(test_dialogue_register_and_fetch, "register and fetch dialogue entries")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_line fetched_line;
    ft_vector<int> next_ids;

    next_ids.push_back(2);
    next_ids.push_back(3);
    build_line(1, "npc", "hello", next_ids, line);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(line));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched_line));
    FT_ASSERT_EQ(1, fetched_line.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), fetched_line.get_speaker());
    FT_ASSERT_EQ(ft_string("hello"), fetched_line.get_text());
    FT_ASSERT_EQ(2, fetched_line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(3, fetched_line.get_next_line_ids()[1]);
    return (1);
}

FT_TEST(test_dialogue_script_isolation, "scripts fetch as copies")
{
    ft_dialogue_table table;
    ft_dialogue_script script;
    ft_dialogue_script fetched_first;
    ft_dialogue_script fetched_second;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_vector<int> empty_next;

    lines.push_back(build_line_shared(10, "guide", "welcome", empty_next));
    script = ft_dialogue_script(5, ft_string("intro"), ft_string("start"), 10, lines);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched_first));
    fetched_first.get_lines()[0]->set_text(ft_string("changed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched_second));
    FT_ASSERT_EQ(ft_string("welcome"), fetched_second.get_lines()[0].get_text());
    return (1);
}

FT_TEST(test_dialogue_missing_entries, "missing ids propagate errors")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(99, line));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_script(42, script));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}
