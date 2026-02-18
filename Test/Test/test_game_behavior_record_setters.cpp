#include "../test_internal.hpp"
#include "../../Game/ft_behavior_action.hpp"
#include "../../Game/ft_behavior_profile.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_behavior_action_setters, "setters update action values and preserve error")
{
    ft_behavior_action action;

    action.set_action_id(15);
    action.set_weight(0.95);
    action.set_cooldown_seconds(7.25);
    FT_ASSERT_EQ(15, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.95, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(7.25, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_setters, "setters replace profile fields and action list")
{
    ft_vector<ft_behavior_action> original_actions;
    ft_vector<ft_behavior_action> new_actions;
    ft_behavior_profile profile;

    original_actions.push_back(ft_behavior_action(5, 0.5, 1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(21, 0.1, 0.9, original_actions));
    new_actions.push_back(ft_behavior_action(7, 0.7, 2.5));
    new_actions.push_back(ft_behavior_action(9, 0.9, 3.5));
    profile.set_profile_id(22);
    profile.set_aggression_weight(0.8);
    profile.set_caution_weight(0.2);
    profile.set_actions(new_actions);
    FT_ASSERT_EQ(22, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.8, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.2, profile.get_caution_weight());
    FT_ASSERT_EQ(2u, profile.get_actions().size());
    FT_ASSERT_EQ(7, profile.get_actions()[0].get_action_id());
    FT_ASSERT_EQ(9, profile.get_actions()[1].get_action_id());
    return (1);
}
