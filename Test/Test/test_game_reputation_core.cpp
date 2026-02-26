#include "../test_internal.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Template/map.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_reputation_milestone_updates, "ft_reputation milestones can be set and retrieved")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_milestone(1, 10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(10, reputation.get_milestone(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_milestone(1, 25);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(25, reputation.get_milestone(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_milestone(99));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_rep_tracking, "ft_reputation reps slot values update correctly")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_rep(2, 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(5, reputation.get_rep(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_rep(2, 12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(12, reputation.get_rep(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_rep(-1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_totals_reflect_changes, "ft_reputation totals update when current changes")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_total_rep(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_current_rep(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.add_current_rep(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(13, reputation.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(7, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.add_current_rep(-5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(8, reputation.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(2, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.sub_total_rep(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(7, reputation.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.sub_current_rep(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(1, reputation.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_map_replacements, "ft_reputation set_milestones/set_reps override prior data")
{
    ft_reputation reputation;
    ft_map<int, int> first_set;
    ft_map<int, int> second_set;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_set.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_set.initialize());
    first_set.insert(1, 4);
    second_set.insert(5, 8);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    reputation.set_milestones(first_set);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(4, reputation.get_milestone(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_milestone(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());

    reputation.set_milestones(second_set);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_milestone(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(8, reputation.get_milestone(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());

    reputation.set_reps(first_set);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(4, reputation.get_rep(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());

    reputation.set_reps(second_set);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(0, reputation.get_rep(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(8, reputation.get_rep(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}
