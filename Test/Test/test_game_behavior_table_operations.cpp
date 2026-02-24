#include "../test_internal.hpp"
#include "../../Game/game_behavior_table.hpp"
#include "../../Game/game_behavior_profile.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int build_profile(ft_behavior_profile &profile, int profile_id,
        double aggression, double caution, int action_id)
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_action action_entry;

    if (action_entry.initialize(action_id, 1.25, 2.5) != FT_ERR_SUCCESS)
        return (action_entry.get_error());
    actions.push_back(action_entry);
    return (profile.initialize(profile_id, aggression, caution, actions));
}

FT_TEST(test_behavior_table_register_and_fetch_profile,
        "register_profile stores profile then fetch_profile retrieves it")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;
    ft_behavior_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, build_profile(profile, 1, 0.4, 0.6, 11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(1, fetched));
    FT_ASSERT_EQ(1, fetched.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_table_fetch_missing_profile_returns_not_found,
        "fetch_profile reports not found when no matching id exists")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_profile(99, fetched));
    return (1);
}

FT_TEST(test_behavior_table_register_updates_entries,
        "register_profile overwrites an entry sharing the identifier")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;
    ft_behavior_profile profile_one;
    ft_behavior_profile profile_two;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, build_profile(profile_one, 2, 0.1, 0.1, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile_one));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, build_profile(profile_two, 2, 0.9, 0.2, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile_two));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(2, fetched));
    FT_ASSERT_DOUBLE_EQ(0.9, fetched.get_aggression_weight());
    return (1);
}

FT_TEST(test_behavior_table_thread_safety_toggle,
        "enable/disable thread safety around registration succeeds")
{
    ft_behavior_table table;
    ft_behavior_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT(table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, build_profile(profile, 7, 0.25, 0.25, 9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT(!table.is_thread_safe());
    return (1);
}
