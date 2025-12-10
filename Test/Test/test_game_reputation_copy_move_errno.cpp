#include "../../Game/game_reputation.hpp"
#include "../../Template/map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

static void set_errno_from_reputation(int error_code)
{
    ft_reputation reputation;

    ft_errno = error_code;
    if (error_code == FT_ER_SUCCESSS)
    {
        reputation.set_total_rep(0);
        return ;
    }
    if (error_code == FT_ERR_INVALID_ARGUMENT)
    {
        reputation.get_milestone(-1);
        return ;
    }
    reputation.get_milestone(9999);
    return ;
}

FT_TEST(test_game_reputation_copy_constructor_transfers_values, "ft_reputation copy constructor copies state")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation copy;

    milestones.insert(1, 10);
    reps.insert(2, 5);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(7);
    source.set_current_rep(3);
    copy = ft_reputation(source);
    FT_ASSERT_EQ(10, copy.get_milestone(1));
    FT_ASSERT_EQ(5, copy.get_rep(2));
    FT_ASSERT_EQ(7, copy.get_total_rep());
    FT_ASSERT_EQ(3, copy.get_current_rep());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    return (1);
}

FT_TEST(test_game_reputation_copy_constructor_preserves_source, "ft_reputation copy keeps original intact")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation copy;

    milestones.insert(4, 8);
    reps.insert(6, 12);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(9);
    source.set_current_rep(2);
    copy = ft_reputation(source);
    FT_ASSERT_EQ(8, source.get_milestone(4));
    FT_ASSERT_EQ(12, source.get_rep(6));
    FT_ASSERT_EQ(9, source.get_total_rep());
    FT_ASSERT_EQ(2, source.get_current_rep());
    return (1);
}

FT_TEST(test_game_reputation_move_constructor_transfers_values, "ft_reputation move constructor moves state")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation moved;

    milestones.insert(3, 15);
    reps.insert(9, 20);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(11);
    source.set_current_rep(5);
    moved = ft_reputation(ft_move(source));
    FT_ASSERT_EQ(15, moved.get_milestone(3));
    FT_ASSERT_EQ(20, moved.get_rep(9));
    FT_ASSERT_EQ(11, moved.get_total_rep());
    FT_ASSERT_EQ(5, moved.get_current_rep());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved.get_error());
    return (1);
}

FT_TEST(test_game_reputation_move_constructor_clears_source, "ft_reputation move constructor clears origin")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation moved;

    milestones.insert(7, 21);
    reps.insert(8, 18);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(14);
    source.set_current_rep(6);
    moved = ft_reputation(ft_move(source));
    FT_ASSERT_EQ(0, source.get_total_rep());
    FT_ASSERT_EQ(0, source.get_current_rep());
    FT_ASSERT_EQ(0, source.get_milestones().size());
    FT_ASSERT_EQ(0, source.get_reps().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_game_reputation_copy_assignment_transfers_entries, "ft_reputation copy assignment duplicates data")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation destination;

    milestones.insert(10, 30);
    reps.insert(20, 40);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(25);
    source.set_current_rep(13);
    destination = source;
    FT_ASSERT_EQ(30, destination.get_milestone(10));
    FT_ASSERT_EQ(40, destination.get_rep(20));
    FT_ASSERT_EQ(25, destination.get_total_rep());
    FT_ASSERT_EQ(13, destination.get_current_rep());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    return (1);
}

FT_TEST(test_game_reputation_copy_assignment_sets_errno_success, "ft_reputation copy assignment sets errno to success")
{
    ft_reputation source;
    ft_reputation destination;

    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    destination = source;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    return (1);
}

FT_TEST(test_game_reputation_move_assignment_transfers_and_clears, "ft_reputation move assignment moves state")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation destination;

    milestones.insert(11, 33);
    reps.insert(22, 44);
    source.set_milestones(milestones);
    source.set_reps(reps);
    source.set_total_rep(19);
    source.set_current_rep(17);
    destination = ft_move(source);
    FT_ASSERT_EQ(33, destination.get_milestone(11));
    FT_ASSERT_EQ(44, destination.get_rep(22));
    FT_ASSERT_EQ(19, destination.get_total_rep());
    FT_ASSERT_EQ(17, destination.get_current_rep());
    FT_ASSERT_EQ(0, source.get_total_rep());
    FT_ASSERT_EQ(0, source.get_current_rep());
    FT_ASSERT_EQ(0, source.get_milestones().size());
    FT_ASSERT_EQ(0, source.get_reps().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_game_reputation_move_assignment_sets_errno_success, "ft_reputation move assignment sets errno to success")
{
    ft_reputation source;
    ft_reputation destination;

    set_errno_from_reputation(FT_ERR_NOT_FOUND);
    destination = ft_move(source);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    return (1);
}

FT_TEST(test_game_reputation_self_copy_assignment_noop, "ft_reputation self copy assignment keeps data")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation reputation;

    milestones.insert(12, 48);
    reps.insert(15, 52);
    reputation.set_milestones(milestones);
    reputation.set_reps(reps);
    reputation.set_total_rep(26);
    reputation.set_current_rep(14);
    reputation = reputation;
    FT_ASSERT_EQ(48, reputation.get_milestone(12));
    FT_ASSERT_EQ(52, reputation.get_rep(15));
    FT_ASSERT_EQ(26, reputation.get_total_rep());
    FT_ASSERT_EQ(14, reputation.get_current_rep());
    return (1);
}

FT_TEST(test_game_reputation_self_move_assignment_noop, "ft_reputation self move assignment stable")
{
    ft_reputation reputation;

    reputation.set_total_rep(31);
    reputation.set_current_rep(7);
    reputation = ft_move(reputation);
    FT_ASSERT_EQ(31, reputation.get_total_rep());
    FT_ASSERT_EQ(7, reputation.get_current_rep());
    return (1);
}

FT_TEST(test_game_reputation_total_getter_sets_errno_success, "ft_reputation::get_total_rep sets errno to success")
{
    ft_reputation reputation;

    reputation.set_total_rep(6);
    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(6, reputation.get_total_rep());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_current_getter_sets_errno_success, "ft_reputation::get_current_rep sets errno to success")
{
    ft_reputation reputation;

    reputation.set_current_rep(4);
    set_errno_from_reputation(FT_ERR_NOT_FOUND);
    FT_ASSERT_EQ(4, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_milestone_sets_errno_success, "ft_reputation::get_milestone sets errno to success")
{
    ft_map<int, int> milestones;
    ft_reputation reputation;

    milestones.insert(25, 60);
    reputation.set_milestones(milestones);
    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(60, reputation.get_milestone(25));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_milestone_invalid_sets_error, "ft_reputation::get_milestone invalid id sets error")
{
    ft_reputation reputation;

    set_errno_from_reputation(FT_ER_SUCCESSS);
    FT_ASSERT_EQ(0, reputation.get_milestone(-3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_milestone_missing_sets_not_found, "ft_reputation::get_milestone missing entry sets error")
{
    ft_reputation reputation;

    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(0, reputation.get_milestone(90));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_rep_sets_errno_success, "ft_reputation::get_rep sets errno to success")
{
    ft_map<int, int> reps;
    ft_reputation reputation;

    reps.insert(32, 70);
    reputation.set_reps(reps);
    set_errno_from_reputation(FT_ERR_NOT_FOUND);
    FT_ASSERT_EQ(70, reputation.get_rep(32));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_rep_invalid_sets_error, "ft_reputation::get_rep invalid id sets error")
{
    ft_reputation reputation;

    set_errno_from_reputation(FT_ER_SUCCESSS);
    FT_ASSERT_EQ(0, reputation.get_rep(-5));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_rep_missing_sets_not_found, "ft_reputation::get_rep missing entry sets errno")
{
    ft_reputation reputation;

    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(0, reputation.get_rep(77));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_error_sets_errno_from_state, "ft_reputation::get_error sets errno based on state")
{
    ft_reputation reputation;

    reputation.get_milestone(90);
    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_error_str_sets_errno_from_state, "ft_reputation::get_error_str sets errno and returns string")
{
    ft_reputation reputation;

    reputation.get_rep(-5);
    set_errno_from_reputation(FT_ERR_NOT_FOUND);
    FT_ASSERT_STR_EQ("Invalid argument", reputation.get_error_str());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_set_milestones_sets_errno_success, "ft_reputation::set_milestones sets errno to success")
{
    ft_map<int, int> milestones;
    ft_reputation reputation;

    milestones.insert(100, 200);
    set_errno_from_reputation(FT_ERR_INVALID_ARGUMENT);
    reputation.set_milestones(milestones);
    FT_ASSERT_EQ(200, reputation.get_milestone(100));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_set_reps_sets_errno_success, "ft_reputation::set_reps sets errno to success")
{
    ft_map<int, int> reps;
    ft_reputation reputation;

    reps.insert(120, 220);
    set_errno_from_reputation(FT_ERR_NOT_FOUND);
    reputation.set_reps(reps);
    FT_ASSERT_EQ(220, reputation.get_rep(120));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

