#include "../../Game/ft_behavior_action.hpp"
#include "../../Game/ft_behavior_profile.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_behavior_action_default_construction_resets_errno_to_success,
    "default constructed behavior action resets errno to success")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_behavior_action action;
    ft_behavior_action action_with_error_state;

    FT_ASSERT_EQ(0, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, action.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    FT_ASSERT_STR_EQ("Operation successful", action.get_error_str());
    FT_ASSERT_EQ(ER_SUCCESS, action_with_error_state.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_action_parameterized_construction_resets_errno_to_success,
    "parameterized behavior action resets errno to success")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_behavior_action action(12, 1.25, 3.5);

    FT_ASSERT_EQ(12, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.25, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(3.5, action.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_action_set_action_id_resets_errno,
    "set_action_id resets ft_errno to success")
{
    ft_behavior_action action;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    action.set_action_id(44);
    FT_ASSERT_EQ(44, action.get_action_id());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_set_weight_resets_errno,
    "set_weight resets ft_errno to success")
{
    ft_behavior_action action;

    ft_errno = FT_ERR_INVALID_POINTER;
    action.set_weight(0.75);
    FT_ASSERT_DOUBLE_EQ(0.75, action.get_weight());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_set_cooldown_resets_errno,
    "set_cooldown_seconds resets ft_errno to success")
{
    ft_behavior_action action;

    ft_errno = FT_ERR_ALREADY_EXISTS;
    action.set_cooldown_seconds(6.25);
    FT_ASSERT_DOUBLE_EQ(6.25, action.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_get_action_id_resets_errno,
    "get_action_id resets errno to success while returning stored value")
{
    ft_behavior_action action(8, 0.5, 1.0);

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(8, action.get_action_id());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_get_weight_resets_errno,
    "get_weight resets errno to success while returning stored value")
{
    ft_behavior_action action(2, 4.5, 1.0);

    ft_errno = FT_ERR_EMPTY;
    FT_ASSERT_DOUBLE_EQ(4.5, action.get_weight());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_get_cooldown_resets_errno,
    "get_cooldown_seconds resets errno to success while returning stored value")
{
    ft_behavior_action action(2, 1.5, 9.0);

    ft_errno = FT_ERR_FULL;
    FT_ASSERT_DOUBLE_EQ(9.0, action.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, action.get_error());
    return (1);
}

FT_TEST(test_behavior_action_move_constructor_resets_errno_to_success,
    "move construction transfers data, resets source, and sets errno to success")
{
    ft_behavior_action source(9, 4.0, 8.0);

    ft_errno = FT_ERR_CONFIGURATION;
    ft_behavior_action moved(ft_move(source));
    FT_ASSERT_EQ(9, moved.get_action_id());
    FT_ASSERT_DOUBLE_EQ(4.0, moved.get_weight());
    FT_ASSERT_DOUBLE_EQ(8.0, moved.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}

FT_TEST(test_behavior_action_copy_assignment_resets_errno_to_success,
    "copy assignment clones values and resets errno to success")
{
    ft_behavior_action source(5, 0.9, 7.5);
    ft_behavior_action destination;

    ft_errno = FT_ERR_TERMINATED;
    destination = source;
    FT_ASSERT_EQ(5, destination.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.9, destination.get_weight());
    FT_ASSERT_DOUBLE_EQ(7.5, destination.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_behavior_action_move_assignment_resets_errno,
    "move assignment transfers values, clears source, and resets errno")
{
    ft_behavior_action source(6, 1.4, 2.2);
    ft_behavior_action destination;

    ft_errno = FT_ERR_PRIORITY_QUEUE_EMPTY;
    destination = ft_move(source);
    FT_ASSERT_EQ(6, destination.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.4, destination.get_weight());
    FT_ASSERT_DOUBLE_EQ(2.2, destination.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_default_construction_resets_errno_to_success,
    "default constructed behavior profile resets errno to success")
{
    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    ft_behavior_profile profile;

    FT_ASSERT_EQ(0, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.0, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, profile.get_caution_weight());
    FT_ASSERT_EQ(true, profile.get_actions().empty());
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_profile_parameterized_construction_resets_errno_to_success,
    "parameterized behavior profile resets errno to success")
{
    ft_vector<ft_behavior_action> actions;

    actions.push_back(ft_behavior_action(10, 0.5, 1.5));
    actions.push_back(ft_behavior_action(11, 0.6, 2.5));
    ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
    ft_behavior_profile profile(42, 0.8, 0.2, actions);
    FT_ASSERT_EQ(42, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.8, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.2, profile.get_caution_weight());
    FT_ASSERT_EQ(2u, profile.get_actions().size());
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_profile_set_profile_id_resets_errno,
    "set_profile_id resets ft_errno to success")
{
    ft_behavior_profile profile;

    ft_errno = FT_ERR_SOCKET_RESOLVE_FAIL;
    profile.set_profile_id(77);
    FT_ASSERT_EQ(77, profile.get_profile_id());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_set_aggression_resets_errno,
    "set_aggression_weight resets ft_errno to success")
{
    ft_behavior_profile profile;

    ft_errno = FT_ERR_SOCKET_RESOLVE_FAMILY;
    profile.set_aggression_weight(0.35);
    FT_ASSERT_DOUBLE_EQ(0.35, profile.get_aggression_weight());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_set_caution_resets_errno,
    "set_caution_weight resets ft_errno to success")
{
    ft_behavior_profile profile;

    ft_errno = FT_ERR_SOCKET_RESOLVE_SERVICE;
    profile.set_caution_weight(0.65);
    FT_ASSERT_DOUBLE_EQ(0.65, profile.get_caution_weight());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_set_actions_resets_errno_and_copies,
    "set_actions copies provided actions and resets errno to success")
{
    ft_behavior_profile profile;
    ft_vector<ft_behavior_action> actions;

    actions.push_back(ft_behavior_action(3, 0.2, 1.1));
    actions.push_back(ft_behavior_action(4, 0.4, 1.2));
    ft_errno = FT_ERR_SOCKET_RESOLVE_OVERFLOW;
    profile.set_actions(actions);
    FT_ASSERT_EQ(2u, profile.get_actions().size());
    FT_ASSERT_EQ(3, profile.get_actions()[0].get_action_id());
    FT_ASSERT_EQ(4, profile.get_actions()[1].get_action_id());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, profile.get_error());
    actions[0].set_action_id(99);
    FT_ASSERT_EQ(3, profile.get_actions()[0].get_action_id());
    return (1);
}

FT_TEST(test_behavior_profile_copy_assignment_resets_errno,
    "copy assignment clones profile values and resets errno to success")
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile source;
    ft_behavior_profile destination;

    actions.push_back(ft_behavior_action(1, 0.1, 0.5));
    actions.push_back(ft_behavior_action(2, 0.2, 0.6));
    source = ft_behavior_profile(5, 0.9, 0.4, actions);
    ft_errno = FT_ERR_SOCKET_RESOLVE_MEMORY;
    destination = source;
    FT_ASSERT_EQ(5, destination.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.9, destination.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.4, destination.get_caution_weight());
    FT_ASSERT_EQ(2u, destination.get_actions().size());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    source.set_profile_id(8);
    FT_ASSERT_EQ(5, destination.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_profile_move_assignment_resets_errno,
    "move assignment transfers data, resets source, and resets errno")
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile source;
    ft_behavior_profile destination;

    actions.push_back(ft_behavior_action(20, 0.7, 1.3));
    source = ft_behavior_profile(14, 0.6, 0.4, actions);
    ft_errno = FT_ERR_SOCKET_RESOLVE_NO_NAME;
    destination = ft_move(source);
    FT_ASSERT_EQ(14, destination.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.6, destination.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.4, destination.get_caution_weight());
    FT_ASSERT_EQ(1u, destination.get_actions().size());
    FT_ASSERT_EQ(0, source.get_profile_id());
    FT_ASSERT_EQ(true, source.get_actions().empty());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}
