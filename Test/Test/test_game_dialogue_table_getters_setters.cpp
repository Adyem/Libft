#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

static void build_dialogue_entries(ft_dialogue_line &line, ft_dialogue_script &script)
{
    ft_vector<int> followups;
    ft_vector<ft_dialogue_line> script_lines;

    followups.push_back(4);
    line = ft_dialogue_line(3, ft_string("ally"), ft_string("welcome"), followups);
    script_lines.push_back(line);
    followups.clear();
    script_lines.push_back(ft_dialogue_line(4, ft_string("ally"), ft_string("farewell"), followups));
    script = ft_dialogue_script(6, ft_string("greeting"), ft_string("intro"), 3, script_lines);
    return ;
}

FT_TEST(test_dialogue_table_register_line_sets_success, "register line stores entry and sets success error")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_dialogue_line fetched;

    build_dialogue_entries(line, script);
    ft_errno = FT_ERR_INVALID_STATE;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.register_line(line));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.fetch_line(3, fetched));
    FT_ASSERT_EQ(ft_string("welcome"), fetched.get_text());
    return (1);
}

FT_TEST(test_dialogue_table_register_script_sets_success, "register script stores entry and sets success error")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_dialogue_script fetched;

    build_dialogue_entries(line, script);
    ft_errno = FT_ERR_INVALID_STATE;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.register_script(script));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.fetch_script(6, fetched));
    FT_ASSERT_EQ(ft_string("greeting"), fetched.get_title());
    return (1);
}

FT_TEST(test_dialogue_table_fetch_line_missing_sets_not_found, "missing line updates error to not found and errno")
{
    ft_dialogue_table table;
    ft_dialogue_line missing;

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(90, missing));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_dialogue_table_fetch_script_missing_sets_not_found, "missing script updates error to not found and errno")
{
    ft_dialogue_table table;
    ft_dialogue_script missing;

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_script(45, missing));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_dialogue_table_set_lines_replaces_entries, "set_lines replaces stored map and clears previous error")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_map<int, ft_dialogue_line> lines;
    ft_dialogue_line fetched;

    build_dialogue_entries(line, script);
    table.register_line(line);
    table.fetch_line(99, fetched);
    lines.insert(10, ft_dialogue_line(10, ft_string("npc"), ft_string("hello"), ft_vector<int>()));
    ft_errno = FT_ERR_INVALID_STATE;
    table.set_lines(lines);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.fetch_line(10, fetched));
    FT_ASSERT_EQ(ft_string("hello"), fetched.get_text());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_line(3, fetched));
    return (1);
}

FT_TEST(test_dialogue_table_set_scripts_replaces_entries, "set_scripts swaps stored scripts and clears previous error")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_map<int, ft_dialogue_script> scripts;
    ft_dialogue_script fetched;

    build_dialogue_entries(line, script);
    table.register_script(script);
    table.fetch_script(11, fetched);
    scripts.insert(11, ft_dialogue_script(11, ft_string("quest"), ft_string("desc"), 0, ft_vector<ft_dialogue_line>()));
    ft_errno = FT_ERR_INVALID_STATE;
    table.set_scripts(scripts);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.fetch_script(11, fetched));
    FT_ASSERT_EQ(ft_string("quest"), fetched.get_title());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_script(6, fetched));
    return (1);
}

FT_TEST(test_dialogue_table_get_lines_sets_success_errno, "get_lines sets success error code")
{
    ft_dialogue_table table;
    ft_dialogue_line line;
    ft_dialogue_script script;
    ft_map<int, ft_dialogue_line> &lines = table.get_lines();

    build_dialogue_entries(line, script);
    lines.insert(7, line);
    ft_errno = FT_ERR_INVALID_STATE;
    const ft_map<int, ft_dialogue_line> &const_lines = table.get_lines();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(1, const_lines.size());
    return (1);
}

FT_TEST(test_dialogue_table_get_scripts_sets_success_errno, "get_scripts sets success error code")
{
    ft_dialogue_table table;
    ft_dialogue_script script;
    ft_map<int, ft_dialogue_script> &scripts = table.get_scripts();

    script = ft_dialogue_script(1, ft_string("intro"), ft_string("desc"), 0, ft_vector<ft_dialogue_line>());
    scripts.insert(1, script);
    ft_errno = FT_ERR_INVALID_STATE;
    const ft_map<int, ft_dialogue_script> &const_scripts = table.get_scripts();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(1, const_scripts.size());
    return (1);
}

FT_TEST(test_dialogue_table_get_error_str_success, "get_error_str returns success message after valid operations")
{
    ft_dialogue_table table;
    const char *message;

    ft_errno = FT_ERR_INVALID_STATE;
    message = table.get_error_str();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT(ft_strcmp(ft_strerror(FT_ERR_SUCCESSS), message) == 0);
    return (1);
}

FT_TEST(test_dialogue_table_get_error_str_after_failure, "get_error_str reflects last error after failed fetch")
{
    ft_dialogue_table table;
    ft_dialogue_line missing;
    const char *message;

    table.fetch_line(123, missing);
    message = table.get_error_str();
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT(ft_strcmp(ft_strerror(FT_ERR_NOT_FOUND), message) == 0);
    return (1);
}
