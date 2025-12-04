#include "../../Game/ft_dialogue_line.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_dialogue_line_set_next_line_ids_overwrites_previous, "set_next_line_ids replaces existing entries with new collection")
{
    ft_vector<int> initial_next_lines;
    ft_vector<int> replacement_next_lines;
    ft_dialogue_line line;

    initial_next_lines.push_back(2);
    initial_next_lines.push_back(4);
    replacement_next_lines.push_back(9);
    line = ft_dialogue_line(1, ft_string("speaker"), ft_string("text"), initial_next_lines);
    line.set_next_line_ids(replacement_next_lines);
    FT_ASSERT_EQ(1ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(9, line.get_next_line_ids()[0]);
    return (1);
}

FT_TEST(test_dialogue_line_nonconst_getter_allows_modification, "non-const getter exposes modifiable next line list")
{
    ft_dialogue_line line(3, ft_string("npc"), ft_string("hint"), ft_vector<int>());
    ft_vector<int> &editable = line.get_next_line_ids();

    editable.push_back(7);
    editable.push_back(11);
    FT_ASSERT_EQ(2ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(7, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(11, line.get_next_line_ids()[1]);
    return (1);
}

FT_TEST(test_dialogue_line_nonconst_getter_preserves_errno, "non-const getter keeps incoming errno unchanged on success")
{
    ft_dialogue_line line(4, ft_string("ally"), ft_string("guide"), ft_vector<int>());
    ft_vector<int> &editable = line.get_next_line_ids();

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    editable.push_back(13);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(1ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(13, line.get_next_line_ids()[0]);
    return (1);
}

FT_TEST(test_dialogue_line_const_getter_preserves_errno, "const getter restores errno after reading next line ids")
{
    ft_vector<int> ids;
    ft_dialogue_line line;

    ids.push_back(5);
    ids.push_back(6);
    line = ft_dialogue_line(2, ft_string("guide"), ft_string("path"), ids);
    ft_errno = FT_ERR_INVALID_POINTER;
    const ft_vector<int> &readonly = line.get_next_line_ids();
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    FT_ASSERT_EQ(2ul, readonly.size());
    FT_ASSERT_EQ(5, readonly[0]);
    FT_ASSERT_EQ(6, readonly[1]);
    return (1);
}

FT_TEST(test_dialogue_line_copy_assignment_deep_copies_next_ids, "copy assignment duplicates next line ids independently")
{
    ft_vector<int> first_ids;
    ft_vector<int> second_ids;
    ft_dialogue_line source;
    ft_dialogue_line destination;

    first_ids.push_back(1);
    first_ids.push_back(2);
    second_ids.push_back(7);
    source = ft_dialogue_line(9, ft_string("narrator"), ft_string("start"), first_ids);
    destination = ft_dialogue_line(4, ft_string("npc"), ft_string("mid"), second_ids);
    destination = source;
    source.get_next_line_ids().push_back(3);
    FT_ASSERT_EQ(2ul, destination.get_next_line_ids().size());
    FT_ASSERT_EQ(1, destination.get_next_line_ids()[0]);
    FT_ASSERT_EQ(2, destination.get_next_line_ids()[1]);
    FT_ASSERT_EQ(3ul, source.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_dialogue_line_move_assignment_transfers_next_ids, "move assignment transfers ids and clears source")
{
    ft_vector<int> ids;
    ft_dialogue_line source;
    ft_dialogue_line destination;

    ids.push_back(10);
    ids.push_back(20);
    source = ft_dialogue_line(6, ft_string("merchant"), ft_string("offer"), ids);
    destination = ft_dialogue_line(1, ft_string("hero"), ft_string("intro"), ft_vector<int>());
    destination = ft_move(source);
    FT_ASSERT_EQ(6, destination.get_line_id());
    FT_ASSERT_EQ(ft_string("merchant"), destination.get_speaker());
    FT_ASSERT_EQ(ft_string("offer"), destination.get_text());
    FT_ASSERT_EQ(2ul, destination.get_next_line_ids().size());
    FT_ASSERT_EQ(10, destination.get_next_line_ids()[0]);
    FT_ASSERT_EQ(20, destination.get_next_line_ids()[1]);
    FT_ASSERT_EQ(0, source.get_line_id());
    FT_ASSERT_EQ(ft_string(), source.get_speaker());
    FT_ASSERT_EQ(ft_string(), source.get_text());
    FT_ASSERT_EQ(0ul, source.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_dialogue_line_move_assignment_resets_error_state, "move assignment leaves moved-from line reporting success")
{
    ft_vector<int> ids;
    ft_dialogue_line source;
    ft_dialogue_line destination;

    ids.push_back(30);
    source = ft_dialogue_line(12, ft_string("ally"), ft_string("backup"), ids);
    destination = ft_move(source);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_self_move_assignment_keeps_values, "self move assignment leaves dialogue line unchanged")
{
    ft_vector<int> ids;
    ft_dialogue_line line;

    ids.push_back(14);
    line = ft_dialogue_line(22, ft_string("scout"), ft_string("report"), ids);
    line = ft_move(line);
    FT_ASSERT_EQ(22, line.get_line_id());
    FT_ASSERT_EQ(ft_string("scout"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("report"), line.get_text());
    FT_ASSERT_EQ(1ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(14, line.get_next_line_ids()[0]);
    return (1);
}

FT_TEST(test_dialogue_line_multiple_speaker_updates_store_latest, "set_speaker replaces earlier speaker values")
{
    ft_dialogue_line line(1, ft_string("old"), ft_string("text"), ft_vector<int>());

    line.set_speaker(ft_string("middle"));
    line.set_speaker(ft_string("new"));
    FT_ASSERT_EQ(ft_string("new"), line.get_speaker());
    return (1);
}

FT_TEST(test_dialogue_line_multiple_text_updates_store_latest, "set_text overwrites previous dialogue text")
{
    ft_dialogue_line line(2, ft_string("npc"), ft_string("first"), ft_vector<int>());

    line.set_text(ft_string("second"));
    line.set_text(ft_string("final"));
    FT_ASSERT_EQ(ft_string("final"), line.get_text());
    return (1);
}

FT_TEST(test_dialogue_line_multiple_line_id_updates_store_latest, "set_line_id applies newest identifier")
{
    ft_dialogue_line line(3, ft_string("npc"), ft_string("line"), ft_vector<int>());

    line.set_line_id(5);
    line.set_line_id(9);
    FT_ASSERT_EQ(9, line.get_line_id());
    return (1);
}

FT_TEST(test_dialogue_line_get_error_reports_success_after_setters, "get_error returns success after performing updates")
{
    ft_vector<int> ids;
    ft_dialogue_line line(7, ft_string("guide"), ft_string("path"), ft_vector<int>());

    ids.push_back(1);
    ids.push_back(2);
    line.set_line_id(8);
    line.set_speaker(ft_string("mentor"));
    line.set_text(ft_string("direction"));
    line.set_next_line_ids(ids);
    FT_ASSERT_EQ(ER_SUCCESS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_get_error_str_matches_errno, "get_error_str returns string for stored error code")
{
    ft_dialogue_line line(5, ft_string("narrator"), ft_string("description"), ft_vector<int>());

    FT_ASSERT_EQ(ft_strerror(ER_SUCCESS), line.get_error_str());
    return (1);
}

FT_TEST(test_dialogue_line_move_constructor_transfers_ids, "move constructor copies ids and clears original line")
{
    ft_vector<int> ids;
    ft_dialogue_line original;

    ids.push_back(4);
    ids.push_back(6);
    original = ft_dialogue_line(16, ft_string("villain"), ft_string("threat"), ids);
    ft_dialogue_line moved(ft_move(original));
    FT_ASSERT_EQ(16, moved.get_line_id());
    FT_ASSERT_EQ(ft_string("villain"), moved.get_speaker());
    FT_ASSERT_EQ(ft_string("threat"), moved.get_text());
    FT_ASSERT_EQ(2ul, moved.get_next_line_ids().size());
    FT_ASSERT_EQ(4, moved.get_next_line_ids()[0]);
    FT_ASSERT_EQ(6, moved.get_next_line_ids()[1]);
    FT_ASSERT_EQ(0, original.get_line_id());
    FT_ASSERT_EQ(ft_string(), original.get_speaker());
    FT_ASSERT_EQ(ft_string(), original.get_text());
    FT_ASSERT_EQ(0ul, original.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_dialogue_line_copy_constructor_after_mutation_is_independent, "copy constructor deep copies next line ids after edits")
{
    ft_vector<int> ids;
    ft_dialogue_line original;
    ft_dialogue_line copy;

    ids.push_back(18);
    original = ft_dialogue_line(21, ft_string("ally"), ft_string("assist"), ids);
    original.get_next_line_ids().push_back(19);
    copy = ft_dialogue_line(original);
    original.get_next_line_ids().push_back(20);
    FT_ASSERT_EQ(2ul, copy.get_next_line_ids().size());
    FT_ASSERT_EQ(18, copy.get_next_line_ids()[0]);
    FT_ASSERT_EQ(19, copy.get_next_line_ids()[1]);
    FT_ASSERT_EQ(3ul, original.get_next_line_ids().size());
    return (1);
}
