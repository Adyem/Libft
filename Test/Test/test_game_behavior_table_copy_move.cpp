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

    actions.push_back(ft_behavior_action(id * 2, 0.5, 1.5));
    profile = ft_behavior_profile(id, aggression, caution, actions);
    return (table.register_profile(profile));
}

FT_TEST(test_behavior_table_move_semantics, "move constructor and assignment transfer table contents and clear source")
{
    ft_behavior_table source;
    ft_behavior_profile fetched;
    ft_behavior_table moved;
    ft_behavior_table reassigned;
    int register_result;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(register_result = register_profile(source, 20, 0.7, 0.3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.fetch_profile(20, fetched));
    FT_ASSERT_EQ(20, fetched.get_profile_id());

    moved = ft_behavior_table(ft_move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.fetch_profile(20, fetched));
    FT_ASSERT_EQ(true, source.get_profiles().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(reassigned, 25, 0.4, 0.6));
    reassigned = ft_move(moved);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.fetch_profile(20, fetched));
    FT_ASSERT_EQ(true, moved.get_profiles().empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    return (1);
}

FT_TEST(test_behavior_table_copy_constructor_reset_errno,
        "copy constructor clones profiles and resets ft_errno to success")
{
    ft_behavior_table source;
    ft_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(source, 30, 0.8, 0.2));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_profile(99, fetched));

    ft_errno = FT_ERR_ALREADY_INITIALIZED;
    ft_behavior_table copy(source);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.fetch_profile(30, fetched));
    FT_ASSERT_EQ(30, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_copy_assignment_reset_errno,
        "copy assignment replaces profiles and resets ft_errno to success")
{
    ft_behavior_table source;
    ft_behavior_table destination;
    ft_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(source, 44, 0.25, 0.75));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, register_profile(destination, 50, 0.6, 0.4));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_profile(77, fetched));

    ft_errno = FT_ERR_CRYPTO_INVALID_PADDING;
    destination = source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.fetch_profile(44, fetched));
    FT_ASSERT_EQ(44, fetched.get_profile_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.fetch_profile(50, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}
