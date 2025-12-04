#include "../../Game/game_behavior_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

static ft_behavior_profile build_profile(int profile_id, double aggression, double caution,
        int action_id)
{
    ft_vector<ft_behavior_action> actions;

    actions.push_back(ft_behavior_action(action_id, 1.25, 2.5));
    return (ft_behavior_profile(profile_id, aggression, caution, actions));
}

FT_TEST(test_behavior_table_default_state_success, "default behavior table starts empty and success")
{
    ft_behavior_table table;

    FT_ASSERT_EQ(true, table.get_profiles().empty());
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    FT_ASSERT_STR_EQ("Success", table.get_error_str());
    return (1);
}

FT_TEST(test_behavior_table_get_error_preserves_entry_errno, "get_error restores entry errno")
{
    ft_behavior_table table;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_get_error_str_preserves_errno, "get_error_str leaves ft_errno unchanged")
{
    ft_behavior_table table;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_STR_EQ("Success", table.get_error_str());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_get_profiles_sets_success, "get_profiles resets table errno to success")
{
    ft_behavior_table table;

    ft_errno = FT_ERR_EMPTY;
    table.get_profiles();
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_const_get_profiles_sets_success,
    "const get_profiles resets table errno to success")
{
    ft_behavior_table table;
    const ft_behavior_table *const_table;

    const_table = &table;
    ft_errno = FT_ERR_FULL;
    const_table->get_profiles();
    FT_ASSERT_EQ(ER_SUCCESS, const_table->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_set_profiles_replaces_contents, "set_profiles swaps existing entries")
{
    ft_behavior_table table;
    ft_map<int, ft_behavior_profile> profiles;
    ft_behavior_profile fetched;

    table.register_profile(build_profile(5, 0.6, 0.4, 11));
    profiles.insert(9, build_profile(9, 0.2, 0.8, 15));
    table.set_profiles(profiles);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_profile(5, fetched));
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(9, fetched));
    FT_ASSERT_EQ(9, fetched.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_table_set_profiles_restores_errno, "set_profiles restores caller errno")
{
    ft_behavior_table table;
    ft_map<int, ft_behavior_profile> profiles;

    profiles.insert(3, build_profile(3, 0.3, 0.7, 8));
    ft_errno = FT_ERR_INVALID_OPERATION;
    table.set_profiles(profiles);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_behavior_table_register_profile_restores_errno,
    "register_profile restores incoming errno on success")
{
    ft_behavior_table table;

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    FT_ASSERT_EQ(ER_SUCCESS, table.register_profile(build_profile(12, 0.9, 0.1, 4)));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_duplicate_registration_returns_error,
    "register_profile reports already exists without clobbering errno")
{
    ft_behavior_table table;

    table.register_profile(build_profile(2, 0.4, 0.6, 10));
    ft_errno = FT_ERR_IO;
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, table.register_profile(build_profile(2, 0.4, 0.6, 10)));
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, table.get_error());
    return (1);
}

FT_TEST(test_behavior_table_fetch_restores_errno_on_success,
    "fetch_profile restores entry errno when profile exists")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    table.register_profile(build_profile(6, 0.1, 0.9, 7));
    ft_errno = FT_ERR_INTERNAL;
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(6, fetched));
    FT_ASSERT_EQ(FT_ERR_INTERNAL, ft_errno);
    FT_ASSERT_EQ(6, fetched.get_profile_id());
    return (1);
}

FT_TEST(test_behavior_table_fetch_not_found_restores_errno,
    "fetch_profile not found leaves entry errno intact")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    ft_errno = FT_ERR_UNSUPPORTED_TYPE;
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_profile(77, fetched));
    FT_ASSERT_EQ(FT_ERR_UNSUPPORTED_TYPE, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_behavior_table_fetch_returns_copy_isolated,
    "fetched profiles remain isolated from table storage")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;
    ft_behavior_profile second_fetch;

    table.register_profile(build_profile(13, 0.5, 0.5, 16));
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(13, fetched));
    fetched.get_actions()[0].set_weight(9.5);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(13, second_fetch));
    FT_ASSERT_DOUBLE_EQ(1.25, second_fetch.get_actions()[0].get_weight());
    return (1);
}

FT_TEST(test_behavior_table_get_error_after_failure, "get_error exposes last error from fetch")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    table.fetch_profile(33, fetched);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_behavior_table_get_error_str_after_failure,
    "get_error_str exposes not found string and preserves errno")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    ft_errno = FT_ERR_FULL;
    table.fetch_profile(21, fetched);
    FT_ASSERT_STR_EQ("Object not found", table.get_error_str());
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_copy_constructor_preserves_error,
    "copy construction preserves stored profiles and errno")
{
    ft_behavior_table table;
    ft_behavior_table copy;
    ft_behavior_profile fetched;

    table.register_profile(build_profile(18, 0.7, 0.3, 22));
    table.fetch_profile(99, fetched);
    ft_errno = FT_ERR_INVALID_STATE;
    copy = ft_behavior_table(table);
    FT_ASSERT_EQ(ER_SUCCESS, copy.fetch_profile(18, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_copy_assignment_preserves_error,
    "copy assignment replaces data and restores errno")
{
    ft_behavior_table source;
    ft_behavior_table destination;
    ft_behavior_profile fetched;

    source.register_profile(build_profile(25, 0.45, 0.55, 30));
    source.fetch_profile(200, fetched);
    destination.register_profile(build_profile(31, 0.2, 0.8, 40));
    ft_errno = FT_ERR_CONFIGURATION;
    destination = source;
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_profile(25, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_move_constructor_clears_source,
    "move construction transfers profiles and clears origin without altering errno")
{
    ft_behavior_table source;
    ft_behavior_table moved;
    ft_behavior_profile fetched;

    source.register_profile(build_profile(41, 0.33, 0.67, 55));
    source.fetch_profile(404, fetched);
    ft_errno = FT_ERR_TERMINATED;
    moved = ft_behavior_table(ft_move(source));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_profile(41, fetched));
    FT_ASSERT_EQ(true, source.get_profiles().empty());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_TERMINATED, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_move_assignment_clears_source,
    "move assignment transfers profiles and restores errno")
{
    ft_behavior_table source;
    ft_behavior_table destination;
    ft_behavior_profile fetched;

    source.register_profile(build_profile(52, 0.15, 0.85, 61));
    destination.register_profile(build_profile(60, 0.9, 0.1, 70));
    ft_errno = FT_ERR_GAME_GENERAL_ERROR;
    destination = ft_move(source);
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_profile(52, fetched));
    FT_ASSERT_EQ(true, source.get_profiles().empty());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_self_copy_assignment_noop,
    "self copy assignment keeps profiles and errno")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    table.register_profile(build_profile(73, 0.52, 0.48, 81));
    table.fetch_profile(700, fetched);
    ft_errno = FT_ERR_EMPTY;
    table = table;
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(73, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_errno);
    return (1);
}

FT_TEST(test_behavior_table_self_move_assignment_noop,
    "self move assignment keeps profiles and errno")
{
    ft_behavior_table table;
    ft_behavior_profile fetched;

    table.register_profile(build_profile(91, 0.22, 0.78, 99));
    table.fetch_profile(910, fetched);
    ft_errno = FT_ERR_ALREADY_INITIALIZED;
    table = ft_move(table);
    FT_ASSERT_EQ(ER_SUCCESS, table.fetch_profile(91, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    FT_ASSERT_EQ(FT_ERR_ALREADY_INITIALIZED, ft_errno);
    return (1);
}
