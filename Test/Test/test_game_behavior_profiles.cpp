#include "../test_internal.hpp"
#include "../../Game/game_behavior_table.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int assert_action_values(const ft_behavior_action &action, int action_id, double weight,
        double cooldown)
{
    FT_ASSERT_EQ(action_id, action.get_action_id());
    FT_ASSERT_DOUBLE_EQ(weight, action.get_weight());
    FT_ASSERT_DOUBLE_EQ(cooldown, action.get_cooldown_seconds());
    return (1);
}

FT_TEST(test_behavior_register_and_fetch, "register and fetch behavior profile")
{
    ft_behavior_table table;
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile profile;
    ft_behavior_profile fetched;
    ft_behavior_action action_entry;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.initialize(1, 0.6, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.get_error());
    actions.push_back(action_entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.initialize(2, 0.4, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.get_error());
    actions.push_back(action_entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(5, 0.8, 0.2, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(5, fetched));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(5, fetched.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.8, fetched.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(0.2, fetched.get_caution_weight());
    assert_action_values(fetched.get_actions()[0], 1, 0.6, 2.0);
    assert_action_values(fetched.get_actions()[1], 2, 0.4, 5.0);
    return (1);
}

FT_TEST(test_behavior_missing_profile, "missing profile surfaces not found")
{
    ft_behavior_table table;
    ft_behavior_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_profile(99, profile));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_behavior_profile_isolation, "fetched copies remain isolated")
{
    ft_behavior_table table;
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile profile;
    ft_behavior_profile first_fetch;
    ft_behavior_profile second_fetch;
    ft_behavior_action action_entry;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.initialize(10, 1.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry.get_error());
    actions.push_back(action_entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(20, 0.5, 0.5, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(20, first_fetch));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    first_fetch.get_actions()[0].set_weight(2.5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(20, second_fetch));
    FT_ASSERT_DOUBLE_EQ(1.0, second_fetch.get_actions()[0].get_weight());
    return (1);
}
