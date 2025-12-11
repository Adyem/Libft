#include "../../Game/game_reputation.hpp"
#include "../../Template/map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_reputation_negative_milestone_sets_error, "ft_reputation rejects negative milestone id")
{
    ft_reputation reputation;

    ft_errno = FT_ERR_SUCCESSS;
    reputation.set_milestone(-1, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_add_current_rep_updates_totals, "ft_reputation::add_current_rep adjusts totals")
{
    ft_reputation reputation;

    reputation.set_total_rep(5);
    reputation.set_current_rep(2);
    reputation.add_current_rep(3);
    FT_ASSERT_EQ(8, reputation.get_total_rep());
    FT_ASSERT_EQ(5, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_set_milestone_overwrites_value, "ft_reputation::set_milestone updates entries")
{
    ft_reputation reputation;

    reputation.set_milestone(1, 10);
    reputation.set_milestone(1, 25);
    FT_ASSERT_EQ(25, reputation.get_milestone(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_missing_milestone_sets_not_found, "ft_reputation reports missing milestones")
{
    ft_reputation reputation;

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, reputation.get_milestone(3));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_set_rep_rejects_negative_id, "ft_reputation::set_rep rejects invalid ids")
{
    ft_reputation reputation;

    ft_errno = FT_ERR_SUCCESSS;
    reputation.set_rep(-1, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(0, reputation.get_rep(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_get_rep_invalid_id_sets_error, "ft_reputation::get_rep rejects negative ids")
{
    ft_reputation reputation;

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, reputation.get_rep(-2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_reputation_set_rep_inserts_and_updates, "ft_reputation::set_rep inserts and overwrites entries")
{
    ft_reputation reputation;

    reputation.set_rep(2, 3);
    FT_ASSERT_EQ(3, reputation.get_rep(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());

    reputation.set_rep(2, 9);
    FT_ASSERT_EQ(9, reputation.get_rep(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_add_current_rep_allows_negative_values, "ft_reputation::add_current_rep handles negative changes")
{
    ft_reputation reputation;

    reputation.set_total_rep(10);
    reputation.set_current_rep(4);
    reputation.add_current_rep(-3);
    FT_ASSERT_EQ(7, reputation.get_total_rep());
    FT_ASSERT_EQ(1, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_set_reps_replaces_entries, "ft_reputation::set_reps overwrites existing rep table")
{
    ft_reputation reputation;
    ft_map<int, int> initial_reps;
    ft_map<int, int> replacement_reps;

    initial_reps.insert(1, 4);
    replacement_reps.insert(2, 6);

    reputation.set_reps(initial_reps);
    FT_ASSERT_EQ(4, reputation.get_rep(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());

    reputation.set_reps(replacement_reps);
    FT_ASSERT_EQ(6, reputation.get_rep(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_rep(1));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

int test_reputation_subtracters(void)
{
    ft_reputation rep;
    rep.set_total_rep(20);
    rep.sub_total_rep(5);
    rep.set_current_rep(10);
    rep.sub_current_rep(3);
    return (rep.get_total_rep() == 12 && rep.get_current_rep() == 7);
}

