#include "../../Game/game_behavior_table.hpp"
#include "../../System_utils/test_runner.hpp"

static int register_profile(ft_behavior_table &table, int id, double aggression, double caution)
{
    ft_vector<ft_behavior_action> actions;
    ft_behavior_profile profile;

    actions.push_back(ft_behavior_action(id * 2, 0.5, 1.5));
    profile = ft_behavior_profile(id, aggression, caution, actions);
    return (table.register_profile(profile));
}

FT_TEST(test_behavior_table_copy_semantics, "copy constructor and assignment clone table entries")
{
    ft_behavior_table original;
    ft_behavior_profile fetched;
    ft_behavior_table copy;
    ft_behavior_table assigned;

    FT_ASSERT_EQ(ER_SUCCESS, register_profile(original, 10, 0.6, 0.4));
    FT_ASSERT_EQ(ER_SUCCESS, register_profile(original, 12, 0.8, 0.2));
    FT_ASSERT_EQ(ER_SUCCESS, original.fetch_profile(99, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, original.get_error());

    copy = ft_behavior_table(original);
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_profile(10, fetched));
    FT_ASSERT_EQ(10, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copy.fetch_profile(77, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copy.get_error());

    assigned = original;
    FT_ASSERT_EQ(ER_SUCCESS, assigned.fetch_profile(12, fetched));
    FT_ASSERT_EQ(12, fetched.get_profile_id());
    FT_ASSERT(copy.get_profiles().find(500) == copy.get_profiles().end());
    return (1);
}

FT_TEST(test_behavior_table_move_semantics, "move constructor and assignment transfer table contents and clear source")
{
    ft_behavior_table source;
    ft_behavior_profile fetched;
    ft_behavior_table moved;
    ft_behavior_table reassigned;

    FT_ASSERT_EQ(ER_SUCCESS, register_profile(source, 20, 0.7, 0.3));
    FT_ASSERT_EQ(ER_SUCCESS, source.fetch_profile(20, fetched));
    FT_ASSERT_EQ(20, fetched.get_profile_id());

    moved = ft_behavior_table(ft_move(source));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_profile(20, fetched));
    FT_ASSERT_EQ(true, source.get_profiles().empty());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());

    FT_ASSERT_EQ(ER_SUCCESS, register_profile(reassigned, 25, 0.4, 0.6));
    reassigned = ft_move(moved);
    FT_ASSERT_EQ(ER_SUCCESS, reassigned.fetch_profile(20, fetched));
    FT_ASSERT_EQ(true, moved.get_profiles().empty());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}
