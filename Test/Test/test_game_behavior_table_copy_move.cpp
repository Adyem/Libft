#include "../test_internal.hpp"
#include "../../Game/game_behavior_table.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int register_profile(ft_behavior_table &table, int id, double aggression, double caution)
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile profile;
    ft_behavior_action action_entry;

    if (action_entry.initialize(id * 2, 0.5, 1.5) != FT_ERR_SUCCESS)
        return (action_entry.get_error());
    actions.push_back(action_entry);
    int initialize_error;

    initialize_error = profile.initialize(id, aggression, caution, actions);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    return (table.register_profile(profile));
}

FT_TEST(test_behavior_table_move_semantics, "move constructor and assignment transfer table contents and clear source")
{
    ft_behavior_table source;
    ft_behavior_profile fetched;
    ft_behavior_table moved;
    ft_behavior_table reassigned;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(source, 20, 0.7, 0.3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_profile(20, fetched));
    FT_ASSERT_EQ(20, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.destroy());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_profile(20, fetched));
    FT_ASSERT_EQ(20, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.destroy());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.initialize(ft_move(moved)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.fetch_profile(20, fetched));
    FT_ASSERT_EQ(20, fetched.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_table_copy_constructor_reset_errno,
        "copy constructor clones profiles and resets ft_errno to success")
{
    ft_behavior_table source;
    ft_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(source, 30, 0.8, 0.2));

    ft_behavior_table copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.fetch_profile(30, fetched));
    FT_ASSERT_EQ(30, fetched.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_table_copy_assignment_reset_errno,
        "copy assignment replaces profiles and resets ft_errno to success")
{
    ft_behavior_table source;
    ft_behavior_table destination;
    ft_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(source, 44, 0.25, 0.75));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(destination, 50, 0.6, 0.4));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_profile(44, fetched));
    FT_ASSERT_EQ(44, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.fetch_profile(50, fetched));
    return (1);
}
