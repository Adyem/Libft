#include "../../Game/ft_behavior_action.hpp"
#include "../../Game/ft_behavior_profile.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_behavior_action_copy_semantics, "copy constructor and assignment clone action fields")
{
    ft_behavior_action original(8, 0.75, 4.0);
    ft_behavior_action copy(original);
    ft_behavior_action assigned;

    FT_ASSERT_EQ(8, copy.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.75, copy.get_weight());
    FT_ASSERT_DOUBLE_EQ(4.0, copy.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copy.get_error());

    assigned = original;
    original.set_weight(2.25);
    FT_ASSERT_EQ(8, assigned.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.75, assigned.get_weight());
    FT_ASSERT_DOUBLE_EQ(4.0, assigned.get_cooldown_seconds());
    FT_ASSERT_DOUBLE_EQ(2.25, original.get_weight());
    return (1);
}

FT_TEST(test_behavior_action_move_semantics, "move constructor and assignment transfer values and reset source")
{
    ft_behavior_action source(3, 0.30, 6.5);
    ft_behavior_action moved(ft_move(source));
    ft_behavior_action reassigned;

    FT_ASSERT_EQ(3, moved.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.30, moved.get_weight());
    FT_ASSERT_DOUBLE_EQ(6.5, moved.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());

    source = ft_behavior_action(9, 1.10, 1.5);
    reassigned = ft_move(source);
    FT_ASSERT_EQ(9, reassigned.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.10, reassigned.get_weight());
    FT_ASSERT_DOUBLE_EQ(1.5, reassigned.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());
    return (1);
}

static int assert_profile_values(const ft_behavior_profile &profile, int id, double aggression,
        double caution, int first_action_id)
{
    FT_ASSERT_EQ(id, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(aggression, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(caution, profile.get_caution_weight());
    FT_ASSERT_EQ(first_action_id, profile.get_actions()[0].get_action_id());
    return (1);
}

FT_TEST(test_behavior_profile_copy_semantics, "copy constructor and assignment duplicate profiles and actions")
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile original;
    ft_behavior_profile copy;
    ft_behavior_profile assigned;

    actions.push_back(ft_behavior_action(4, 0.4, 2.0));
    actions.push_back(ft_behavior_action(6, 0.6, 3.0));
    original = ft_behavior_profile(12, 0.7, 0.3, actions);
    copy = ft_behavior_profile(original);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copy.get_error());
    assert_profile_values(copy, 12, 0.7, 0.3, 4);

    assigned = original;
    original.get_actions()[0].set_action_id(99);
    assert_profile_values(assigned, 12, 0.7, 0.3, 4);
    assert_profile_values(original, 12, 0.7, 0.3, 99);
    return (1);
}

FT_TEST(test_behavior_profile_move_semantics, "move constructor and assignment transfer profile state and clear source")
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile source;
    ft_behavior_profile moved;
    ft_behavior_profile reassigned;

    actions.push_back(ft_behavior_action(1, 0.2, 1.0));
    source = ft_behavior_profile(30, 0.9, 0.1, actions);
    moved = ft_behavior_profile(ft_move(source));
    assert_profile_values(moved, 30, 0.9, 0.1, 1);
    FT_ASSERT_EQ(0, source.get_profile_id());
    FT_ASSERT_EQ(true, source.get_actions().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());

    actions.clear();
    actions.push_back(ft_behavior_action(2, 0.3, 4.0));
    actions.push_back(ft_behavior_action(3, 0.7, 5.0));
    source = ft_behavior_profile(44, 0.55, 0.45, actions);
    reassigned = ft_move(source);
    assert_profile_values(reassigned, 44, 0.55, 0.45, 2);
    FT_ASSERT_EQ(true, source.get_actions().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());
    return (1);
}
