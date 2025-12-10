#include "../../Game/ft_dialogue_line.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_dialogue_line_default_initializes_success, "default constructor initializes values with success error")
{
    ft_dialogue_line line;

    FT_ASSERT_EQ(0, line.get_line_id());
    FT_ASSERT_EQ(ft_string(), line.get_speaker());
    FT_ASSERT_EQ(ft_string(), line.get_text());
    FT_ASSERT_EQ(0ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_parameterized_stores_fields, "parameterized constructor stores provided dialogue fields")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line;

    next_lines.push_back(5);
    next_lines.push_back(8);
    line = ft_dialogue_line(3, ft_string("npc"), ft_string("hello"), next_lines);
    FT_ASSERT_EQ(3, line.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("hello"), line.get_text());
    FT_ASSERT_EQ(2ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(5, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(8, line.get_next_line_ids()[1]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_set_line_id_sets_errno_success, "set_line_id updates identifier and clears errno on success")
{
    ft_dialogue_line line(1, ft_string("ally"), ft_string("start"), ft_vector<int>());

    ft_errno = FT_ERR_TERMINATED;
    line.set_line_id(7);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(7, line.get_line_id());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_set_speaker_sets_errno_success, "set_speaker updates value while setting errno to success")
{
    ft_dialogue_line line(1, ft_string("ally"), ft_string("start"), ft_vector<int>());

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    line.set_speaker(ft_string("guide"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(ft_string("guide"), line.get_speaker());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_set_text_sets_errno_success, "set_text updates dialogue text while setting errno to success")
{
    ft_dialogue_line line(1, ft_string("ally"), ft_string("start"), ft_vector<int>());

    ft_errno = FT_ERR_MUTEX_NOT_OWNER;
    line.set_text(ft_string("updated"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(ft_string("updated"), line.get_text());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_set_next_line_ids_copies_values_and_clears_errno, "set_next_line_ids copies entries and resets errno to success")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line(1, ft_string("ally"), ft_string("start"), ft_vector<int>());

    next_lines.push_back(2);
    next_lines.push_back(4);
    ft_errno = FT_ERR_END_OF_FILE;
    line.set_next_line_ids(next_lines);
    next_lines.push_back(6);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(2ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(2, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(4, line.get_next_line_ids()[1]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_copy_constructor_transfers_state, "copy constructor duplicates dialogue fields and error")
{
    ft_vector<int> next_lines;
    ft_dialogue_line original;
    ft_dialogue_line copy;

    next_lines.push_back(9);
    original = ft_dialogue_line(4, ft_string("npc"), ft_string("question"), next_lines);
    copy = ft_dialogue_line(original);
    FT_ASSERT_EQ(4, copy.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), copy.get_speaker());
    FT_ASSERT_EQ(ft_string("question"), copy.get_text());
    FT_ASSERT_EQ(1ul, copy.get_next_line_ids().size());
    FT_ASSERT_EQ(9, copy.get_next_line_ids()[0]);
    FT_ASSERT_EQ(original.get_error(), copy.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_move_constructor_clears_source, "move constructor transfers values and resets source")
{
    ft_vector<int> next_lines;
    ft_dialogue_line original;
    ft_dialogue_line moved;

    next_lines.push_back(11);
    next_lines.push_back(12);
    original = ft_dialogue_line(5, ft_string("npc"), ft_string("farewell"), next_lines);
    moved = ft_dialogue_line(ft_move(original));
    FT_ASSERT_EQ(5, moved.get_line_id());
    FT_ASSERT_EQ(ft_string("npc"), moved.get_speaker());
    FT_ASSERT_EQ(ft_string("farewell"), moved.get_text());
    FT_ASSERT_EQ(2ul, moved.get_next_line_ids().size());
    FT_ASSERT_EQ(11, moved.get_next_line_ids()[0]);
    FT_ASSERT_EQ(12, moved.get_next_line_ids()[1]);
    FT_ASSERT_EQ(0, original.get_line_id());
    FT_ASSERT_EQ(ft_string(), original.get_speaker());
    FT_ASSERT_EQ(ft_string(), original.get_text());
    FT_ASSERT_EQ(0ul, original.get_next_line_ids().size());
    return (1);
}

FT_TEST(test_dialogue_line_self_assignment_resets_errno, "self copy assignment retains dialogue data but resets errno to success")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line;

    next_lines.push_back(15);
    line = ft_dialogue_line(8, ft_string("ally"), ft_string("hint"), next_lines);
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    line = line;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(8, line.get_line_id());
    FT_ASSERT_EQ(ft_string("ally"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("hint"), line.get_text());
    FT_ASSERT_EQ(1ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(15, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}

FT_TEST(test_dialogue_line_getters_reset_errno_to_success, "getters reset errno to success after access")
{
    ft_vector<int> next_lines;
    ft_dialogue_line line;

    next_lines.push_back(21);
    line = ft_dialogue_line(13, ft_string("guide"), ft_string("tip"), next_lines);
    ft_errno = FT_ERR_CONFIGURATION;
    FT_ASSERT_EQ(13, line.get_line_id());
    FT_ASSERT_EQ(ft_string("guide"), line.get_speaker());
    FT_ASSERT_EQ(ft_string("tip"), line.get_text());
    FT_ASSERT_EQ(1ul, line.get_next_line_ids().size());
    FT_ASSERT_EQ(21, line.get_next_line_ids()[0]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, line.get_error());
    return (1);
}
