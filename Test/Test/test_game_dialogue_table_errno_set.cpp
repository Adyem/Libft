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

FT_TEST(test_dialogue_table_register_line_success, "register_line stores a new line and allows fetching")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_dialogue_line stored(1, ft_string("npc"), ft_string("hello"), ft_vector<int>());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(stored));
    ft_dialogue_line fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched));
    FT_ASSERT_EQ(1, fetched.get_line_id());
    return (1);
}

FT_TEST(test_dialogue_table_register_script_success, "register_script stores metadata and lines")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    script.set_title(ft_string("quest"));
    script.set_summary(ft_string("start"));
    script.set_start_line_id(2);
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    lines.push_back(make_line(2, "npc", "reply"));
    script.set_lines(lines);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    ft_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    FT_ASSERT_EQ(5, fetched.get_script_id());
    FT_ASSERT_EQ(ft_string("quest"), fetched.get_title());
    return (1);
}
