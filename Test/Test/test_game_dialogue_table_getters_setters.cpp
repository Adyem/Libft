#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<ft_dialogue_line> build_line(int id, const char *speaker,
        const char *text)
{
    ft_vector<int> next;
    next.push_back(id + 1);
    ft_sharedptr<ft_dialogue_line> stored(new (std::nothrow) ft_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id,
        ft_string(speaker), ft_string(text), next));
    return (stored);
}

FT_TEST(test_dialogue_table_get_lines_accessor_returns_map,
        "get_lines exposes the storage map for direct inspection")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_map<int, ft_sharedptr<ft_dialogue_line>> &lines = table.get_lines();

    ft_sharedptr<ft_dialogue_line> stored_line = build_line(3, "npc", "hello");
    lines.insert(3, stored_line);
    FT_ASSERT_EQ(1, lines.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored_line->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_get_scripts_accessor_returns_map,
        "get_scripts exposes stored scripts map")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_map<int, ft_dialogue_script> &scripts = table.get_scripts();

    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(5);
    scripts.insert(5, script);
    FT_ASSERT_EQ(1, scripts.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_scripts_map_mutation_does_not_spoil_storage,
        "scripts retrieved from map remain independent entries")
{
    ft_dialogue_table table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    ft_map<int, ft_dialogue_script> &scripts = table.get_scripts();
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(7);
    script.set_title(ft_string("intro"));
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    lines.push_back(build_line(7, "npc", "reply"));
    script.set_lines(lines);
    scripts.insert(7, script);

    ft_dialogue_script &stored = scripts.at(7);
    stored.set_title(ft_string("modified"));
    ft_dialogue_script fetched;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(7, fetched));
    FT_ASSERT_NEQ(ft_string("modified"), script.get_title());
    FT_ASSERT_EQ(ft_string("modified"), stored.get_title());
    FT_ASSERT_EQ(ft_string("intro"), fetched.get_title());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines[0]->get_error());
    return (1);
}
