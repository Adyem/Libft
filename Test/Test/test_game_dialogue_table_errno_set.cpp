#include "../test_internal.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void populate_dialogue_records(ft_dialogue_line &line, ft_dialogue_script &script)
{
    ft_vector<int> followups;
    ft_vector<ft_dialogue_line> lines;

    followups.push_back(2);
    line = ft_dialogue_line(1, ft_string("npc"), ft_string("hello"), followups);
    lines.push_back(line);
    followups.clear();
    lines.push_back(ft_dialogue_line(2, ft_string("npc"), ft_string("bye"), followups));
    script = ft_dialogue_script(5, ft_string("intro"), ft_string("start"), 1, lines);
    return ;
}

FT_TEST(test_dialogue_table_register_line_sets_errno_on_success, "register line updates errno to success")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;

    populate_dialogue_records(line, script);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_line(line));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_register_script_sets_errno_on_success, "register script updates errno to success")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;

    populate_dialogue_records(line, script);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_script(script));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_fetch_line_sets_errno_on_success, "fetch line sets errno to success when found")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_line fetched;
    ft_dialogue_script script;

    populate_dialogue_records(line, script);
    table.register_line(line);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(1, fetched.get_line_id());
    return (1);
}

FT_TEST(test_dialogue_table_fetch_script_sets_errno_on_success, "fetch script sets errno to success when found")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_dialogue_script fetched;

    populate_dialogue_records(line, script);
    table.register_script(script);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(ft_string("intro"), fetched.get_title());
    return (1);
}

FT_TEST(test_dialogue_table_fetch_line_returns_copy, "fetch line returns independent copy of stored data")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_line fetched_first;
    ft_dialogue_line fetched_second;
    ft_dialogue_script script;

    populate_dialogue_records(line, script);
    table.register_line(line);
    table.fetch_line(1, fetched_first);
    fetched_first.set_text(ft_string("updated"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(1, fetched_second));
    FT_ASSERT_EQ(ft_string("hello"), fetched_second.get_text());
    return (1);
}

FT_TEST(test_dialogue_table_fetch_script_returns_copy, "fetch script returns independent copy of stored data")
{
    ft_dialogue_table table;
    ft_dialogue_script script;
    ft_dialogue_script fetched_first;
    ft_dialogue_script fetched_second;
    ft_dialogue_line line;

    populate_dialogue_records(line, script);
    table.register_script(script);
    table.fetch_script(5, fetched_first);
    fetched_first.get_lines()[0].set_text(ft_string("changed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_script(5, fetched_second));
    FT_ASSERT_EQ(ft_string("hello"), fetched_second.get_lines()[0].get_text());
    return (1);
}

FT_TEST(test_dialogue_table_move_constructor_sets_errno_success, "move constructor resets errno to success on move")
{
    ft_dialogue_table table;
    ft_dialogue_line missing;
    ft_dialogue_table moved;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(42, missing));
    moved = ft_dialogue_table(ft_move(table));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(true, moved.get_lines().empty());
    FT_ASSERT_EQ(true, moved.get_scripts().empty());
    return (1);
}

FT_TEST(test_dialogue_table_move_assignment_sets_errno_success, "move assignment resets errno to success and clears source")
{
    ft_dialogue_table table;
    ft_dialogue_table destination;
    ft_dialogue_line missing;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(99, missing));
    destination = ft_move(table);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(true, destination.get_lines().empty());
    FT_ASSERT_EQ(true, destination.get_scripts().empty());
    return (1);
}

FT_TEST(test_dialogue_table_copy_constructor_sets_errno_success, "copy constructor resets errno to success")
{
    ft_dialogue_table table;
    ft_dialogue_line missing;
    ft_dialogue_table copied;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(7, missing));
    copied = ft_dialogue_table(table);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_copy_assignment_sets_errno_success, "copy assignment resets errno to success")
{
    ft_dialogue_table table;
    ft_dialogue_table assigned;
    ft_dialogue_line missing;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(8, missing));
    assigned = table;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_dialogue_table_register_line_single_global_error,
    "register_line adds exactly one global stack entry")
{
    ft_dialogue_table table;
    ft_vector<int> next_lines;
    ft_dialogue_line line(15, ft_string("npc"), ft_string("hi"), next_lines);
    int register_result;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(register_result = table.register_line(line));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    return (1);
}
