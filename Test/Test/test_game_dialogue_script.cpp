#include "../test_internal.hpp"
#include "../../CPP_class/class_shared_ptr.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<ft_dialogue_line> create_dialogue_line(int line_id,
        const ft_string &speaker, const ft_string &text)
{
    ft_vector<int> next_lines;

    next_lines.push_back(line_id + 1);
    return (ft_sharedptr<ft_dialogue_line>(new (std::nothrow)
            ft_dialogue_line(line_id, speaker, text, next_lines)));
}

FT_TEST(test_dialogue_script_default_initializes_success,
        "default constructor initializes dialogue script with success")
{
    ft_dialogue_script script;

    FT_ASSERT_EQ(0, script.get_script_id());
    FT_ASSERT_EQ(ft_string(), script.get_title());
    FT_ASSERT_EQ(ft_string(), script.get_summary());
    FT_ASSERT_EQ(0, script.get_start_line_id());
    FT_ASSERT_EQ(0ul, script.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_parameterized_copies_lines,
        "parameterized constructor stores provided fields and lines")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(1, ft_string("npc"),
                ft_string("hi")));
    lines.push_back(create_dialogue_line(3, ft_string("hero"),
                ft_string("bye")));
    script = ft_dialogue_script(7, ft_string("intro"),
            ft_string("greeting"), 1, lines);
    FT_ASSERT_EQ(7, script.get_script_id());
    FT_ASSERT_EQ(ft_string("intro"), script.get_title());
    FT_ASSERT_EQ(ft_string("greeting"), script.get_summary());
    FT_ASSERT_EQ(1, script.get_start_line_id());
    FT_ASSERT_EQ(2ul, script.get_lines().size());
    FT_ASSERT_EQ(1, script.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(3, script.get_lines()[1]->get_line_id());
    return (1);
}

FT_TEST(test_dialogue_script_setters_reset_errno,
        "setters update values and set errno to success")
{
    ft_dialogue_script script(4, ft_string("title"), ft_string("summary"), 10, ft_vector<ft_sharedptr<ft_dialogue_line>>());

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    script.set_script_id(9);
    script.set_title(ft_string("new title"));
    script.set_summary(ft_string("new summary"));
    script.set_start_line_id(12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(9, script.get_script_id());
    FT_ASSERT_EQ(ft_string("new title"), script.get_title());
    FT_ASSERT_EQ(ft_string("new summary"), script.get_summary());
    FT_ASSERT_EQ(12, script.get_start_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_set_lines_copies_entries, "set_lines duplicates provided list without sharing")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(5, ft_string("npc"), ft_string("question")));
    ft_errno = FT_ERR_INTERNAL;
    script.set_lines(lines);
    lines[0]->set_line_id(15);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(1ul, script.get_lines().size());
    FT_ASSERT_EQ(5, script.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_copy_constructor_transfers_state, "copy constructor duplicates dialogue script data")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script original;
    ft_dialogue_script copy;

    lines.push_back(create_dialogue_line(2, ft_string("npc"), ft_string("info")));
    original = ft_dialogue_script(6, ft_string("quest"), ft_string("desc"), 2, lines);
    copy = ft_dialogue_script(original);
    FT_ASSERT_EQ(original.get_script_id(), copy.get_script_id());
    FT_ASSERT_EQ(original.get_title(), copy.get_title());
    FT_ASSERT_EQ(original.get_summary(), copy.get_summary());
    FT_ASSERT_EQ(original.get_start_line_id(), copy.get_start_line_id());
    FT_ASSERT_EQ(original.get_lines().size(), copy.get_lines().size());
    FT_ASSERT_EQ(original.get_lines()[0]->get_line_id(), copy.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(original.get_error(), copy.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_copy_assignment_resets_errno, "copy assignment replaces data and resets errno to success")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_vector<ft_sharedptr<ft_dialogue_line>> other_lines;
    ft_dialogue_script destination;
    ft_dialogue_script source;

    lines.push_back(create_dialogue_line(11, ft_string("npc"), ft_string("talk")));
    other_lines.push_back(create_dialogue_line(20, ft_string("ally"), ft_string("help")));
    destination = ft_dialogue_script(3, ft_string("old"), ft_string("sum"), 11, lines);
    source = ft_dialogue_script(9, ft_string("new"), ft_string("news"), 20, other_lines);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    destination = source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(9, destination.get_script_id());
    FT_ASSERT_EQ(ft_string("new"), destination.get_title());
    FT_ASSERT_EQ(ft_string("news"), destination.get_summary());
    FT_ASSERT_EQ(20, destination.get_start_line_id());
    FT_ASSERT_EQ(1ul, destination.get_lines().size());
    FT_ASSERT_EQ(20, destination.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(source.get_error(), destination.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_move_constructor_clears_source, "move constructor transfers fields and resets moved script")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script original;
    ft_dialogue_script moved;

    lines.push_back(create_dialogue_line(30, ft_string("npc"), ft_string("start")));
    original = ft_dialogue_script(14, ft_string("title"), ft_string("sum"), 30, lines);
    moved = ft_dialogue_script(ft_move(original));
    FT_ASSERT_EQ(14, moved.get_script_id());
    FT_ASSERT_EQ(ft_string("title"), moved.get_title());
    FT_ASSERT_EQ(ft_string("sum"), moved.get_summary());
    FT_ASSERT_EQ(30, moved.get_start_line_id());
    FT_ASSERT_EQ(1ul, moved.get_lines().size());
    FT_ASSERT_EQ(0, original.get_script_id());
    FT_ASSERT_EQ(0, original.get_start_line_id());
    FT_ASSERT_EQ(0ul, original.get_lines().size());
    return (1);
}

FT_TEST(test_dialogue_script_move_assignment_clears_source, "move assignment replaces fields and empties source")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_vector<ft_sharedptr<ft_dialogue_line>> other_lines;
    ft_dialogue_script destination;
    ft_dialogue_script source;

    lines.push_back(create_dialogue_line(8, ft_string("npc"), ft_string("line")));
    other_lines.push_back(create_dialogue_line(12, ft_string("ally"), ft_string("hint")));
    destination = ft_dialogue_script(1, ft_string("dest"), ft_string("sum"), 8, lines);
    source = ft_dialogue_script(5, ft_string("src"), ft_string("summary"), 12, other_lines);
    ft_errno = FT_ERR_INVALID_STATE;
    destination = ft_move(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(5, destination.get_script_id());
    FT_ASSERT_EQ(ft_string("src"), destination.get_title());
    FT_ASSERT_EQ(ft_string("summary"), destination.get_summary());
    FT_ASSERT_EQ(12, destination.get_start_line_id());
    FT_ASSERT_EQ(1ul, destination.get_lines().size());
    FT_ASSERT_EQ(0, source.get_script_id());
    FT_ASSERT_EQ(0ul, source.get_lines().size());
    return (1);
}

FT_TEST(test_dialogue_script_self_assignment_no_change, "self copy assignment leaves script unchanged")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(2, ft_string("npc"), ft_string("talk")));
    script = ft_dialogue_script(10, ft_string("self"), ft_string("assign"), 2, lines);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    script = script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(10, script.get_script_id());
    FT_ASSERT_EQ(ft_string("self"), script.get_title());
    FT_ASSERT_EQ(ft_string("assign"), script.get_summary());
    FT_ASSERT_EQ(2, script.get_start_line_id());
    FT_ASSERT_EQ(1ul, script.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_self_move_assignment_no_change, "self move assignment preserves existing state")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(4, ft_string("npc"), ft_string("note")));
    script = ft_dialogue_script(13, ft_string("keep"), ft_string("move"), 4, lines);
    ft_errno = FT_ERR_INVALID_OPERATION;
    script = ft_move(script);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(13, script.get_script_id());
    FT_ASSERT_EQ(ft_string("keep"), script.get_title());
    FT_ASSERT_EQ(ft_string("move"), script.get_summary());
    FT_ASSERT_EQ(4, script.get_start_line_id());
    FT_ASSERT_EQ(1ul, script.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_getters_reset_errno, "getters reset errno to success after access")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(16, ft_string("npc"), ft_string("line")));
    script = ft_dialogue_script(21, ft_string("title"), ft_string("summary"), 16, lines);
    ft_errno = FT_ERR_NOT_FOUND;
    FT_ASSERT_EQ(21, script.get_script_id());
    FT_ASSERT_EQ(ft_string("title"), script.get_title());
    FT_ASSERT_EQ(ft_string("summary"), script.get_summary());
    FT_ASSERT_EQ(16, script.get_start_line_id());
    FT_ASSERT_EQ(1ul, script.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_dialogue_script_set_lines_single_global_error,
    "set_lines records a single error stack entry")
{
    ft_dialogue_script script;
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_line line(2, ft_string("hero"), ft_string("hello"), ft_vector<int>());

    lines.push_back(line);
    FT_ASSERT_SINGLE_GLOBAL_ERROR(script.set_lines(lines));
    return (1);
}

FT_TEST(test_dialogue_script_non_const_lines_reference, "non-const lines getter allows updates and resets errno")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script script;

    lines.push_back(create_dialogue_line(25, ft_string("npc"), ft_string("text")));
    script = ft_dialogue_script(30, ft_string("mutable"), ft_string("lines"), 25, lines);
    ft_errno = FT_ERR_END_OF_FILE;
    script.get_lines().push_back(create_dialogue_line(40, ft_string("npc"), ft_string("extra")));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(2ul, script.get_lines().size());
    FT_ASSERT_EQ(40, script.get_lines()[1]->get_line_id());
    return (1);
}

FT_TEST(test_dialogue_script_error_reporting_matches_errno, "get_error_str matches stored error code")
{
    ft_dialogue_script script;

    script.set_script_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    FT_ASSERT_EQ(ft_string(ft_strerror(FT_ERR_SUCCESS)), ft_string(script.get_error_str()));
    return (1);
}

FT_TEST(test_dialogue_script_set_lines_replaces_existing_entries, "set_lines replaces previous stored lines")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> first_lines;
    ft_vector<ft_sharedptr<ft_dialogue_line>> second_lines;
    ft_dialogue_script script;

    first_lines.push_back(create_dialogue_line(70, ft_string("npc"), ft_string("first")));
    second_lines.push_back(create_dialogue_line(80, ft_string("npc"), ft_string("second")));
    script = ft_dialogue_script(90, ft_string("title"), ft_string("sum"), 70, first_lines);
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    script.set_lines(second_lines);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(1ul, script.get_lines().size());
    FT_ASSERT_EQ(80, script.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.get_error());
    return (1);
}

FT_TEST(test_dialogue_script_lines_independent_after_copy, "copied scripts retain separate line storage")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script original;
    ft_dialogue_script copy;

    lines.push_back(create_dialogue_line(33, ft_string("npc"), ft_string("first")));
    original = ft_dialogue_script(41, ft_string("title"), ft_string("summary"), 33, lines);
    copy = ft_dialogue_script(original);
    copy.get_lines()[0]->set_line_id(99);
    FT_ASSERT_EQ(33, original.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(99, copy.get_lines()[0]->get_line_id());
    return (1);
}

FT_TEST(test_dialogue_script_lines_independent_after_move, "moved scripts share no references with destination")
{
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    ft_dialogue_script source;
    ft_dialogue_script destination;

    lines.push_back(create_dialogue_line(50, ft_string("npc"), ft_string("origin")));
    source = ft_dialogue_script(60, ft_string("title"), ft_string("sum"), 50, lines);
    destination = ft_move(source);
    destination.get_lines()[0]->set_line_id(75);
    FT_ASSERT_EQ(75, destination.get_lines()[0]->get_line_id());
    FT_ASSERT_EQ(0ul, source.get_lines().size());
    return (1);
}
