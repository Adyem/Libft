#include "../test_internal.hpp"
#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_behavior_action_setters)
{
    game_behavior_action action;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.initialize(0, 0.0, 0.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    action.set_action_id(15);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    action.set_weight(0.95);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    action.set_cooldown_seconds(7.25);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    FT_ASSERT_EQ(15, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.95, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(7.25, action.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_setters)
{
    ft_vector<game_behavior_action> original_actions;
    ft_vector<game_behavior_action> new_actions;
    game_behavior_profile profile;
    game_behavior_action action_entry_first;
    game_behavior_action action_entry_second;
    game_behavior_action action_entry_third;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_actions.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_actions.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.initialize(5, 0.5, 1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.get_error());
    original_actions.push_back(action_entry_first);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(21, 0.1, 0.9, original_actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.initialize(7, 0.7, 2.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.get_error());
    new_actions.push_back(action_entry_second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_third.initialize(9, 0.9, 3.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_third.get_error());
    new_actions.push_back(action_entry_third);
    profile.set_profile_id(22);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_aggression_weight(0.8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_caution_weight(0.2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    profile.set_actions(new_actions);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(22, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.8, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.2, profile.get_caution_weight());
    FT_ASSERT_EQ(2u, profile.get_actions().size());
    FT_ASSERT_EQ(7, profile.get_actions()[0].get_action_id());
    FT_ASSERT_EQ(9, profile.get_actions()[1].get_action_id());
    return (1);
}
