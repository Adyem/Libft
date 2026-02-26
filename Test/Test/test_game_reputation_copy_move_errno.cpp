#include "../test_internal.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Template/map.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_reputation_initialize_copy_retains_state, "ft_reputation::initialize(copy) duplicates state")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, milestones.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reps.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    milestones.insert(7, 21);
    reps.insert(9, 18);
    source.set_milestones(milestones);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_reps(reps);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_total_rep(12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_current_rep(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(21, clone.get_milestone(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(18, clone.get_rep(9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(12, clone.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(6, clone.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}

FT_TEST(test_game_reputation_initialize_move_clears_source,
    "ft_reputation::initialize(move) transfers state and resets source")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation source;
    ft_reputation destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, milestones.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reps.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    milestones.insert(11, 33);
    reps.insert(22, 44);
    source.set_milestones(milestones);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_reps(reps);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_total_rep(19);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    source.set_current_rep(17);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(33, destination.get_milestone(11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(44, destination.get_rep(22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(19, destination.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(17, destination.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(0, source.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(0, source.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(0, source.get_milestones().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(0, source.get_reps().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_game_reputation_accessors_return_values, "ft_reputation getters and setters maintain counts")
{
    ft_map<int, int> milestones;
    ft_map<int, int> reps;
    ft_reputation tracker;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, milestones.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reps.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    milestones.insert(5, 20);
    reps.insert(7, 14);
    tracker.set_milestones(milestones);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    tracker.set_reps(reps);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    tracker.set_total_rep(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    tracker.set_current_rep(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(20, tracker.get_milestone(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(14, tracker.get_rep(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(8, tracker.get_total_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(2, tracker.get_current_rep());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(0, tracker.get_milestone(999));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    FT_ASSERT_EQ(0, tracker.get_rep(999));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tracker.get_error());
    return (1);
}
