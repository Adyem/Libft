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
    return (ft_sharedptr<ft_dialogue_line>(new (std::nothrow) ft_dialogue_line(
            id, ft_string(speaker), ft_string(text), next)));
}

FT_TEST(test_dialogue_table_register_line_retrieves_data,
        "register_line stores line copies accessible by id")
{
    ft_dialogue_table table;
    ft_dialogue_line loaded;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_vector<int> next;
    next.push_back(4);
    ft_dialogue_line original(1, ft_string("npc"), ft_string("hello"), next);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, loaded));
    FT_ASSERT_EQ(1, loaded.get_line_id());
    FT_ASSERT_EQ(ft_string("hello"), loaded.get_text());
    return (1);
}

FT_TEST(test_dialogue_table_scripts_store_lines_via_sharedptrs,
        "register_script accepts metadata and keeps line pointers isolated")
{
    ft_dialogue_table table;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    lines.push_back(make_line(2, "player", "reply"));

    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    script.set_title(ft_string("intro"));
    script.set_summary(ft_string("start"));
    script.set_start_line_id(2);
    script.set_lines(lines);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    ft_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    const ft_vector<ft_sharedptr<ft_dialogue_line>> &fetched_lines = fetched.get_lines();
    FT_ASSERT_EQ(1u, fetched_lines.size());
    FT_ASSERT_EQ(2, fetched_lines[0]->get_line_id());
    return (1);
}
